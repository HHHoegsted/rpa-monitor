#include "publisher.h"

#include <mosquitto.h>
#include <mqtt_protocol.h>

#include <chrono>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>


namespace
{
    constexpr int KEEPALIVE_SECONDS = 30;
    constexpr int LOOP_TIMEOUT_MS = 100;

    constexpr auto ACK_TIMEOUT = std::chrono::seconds(10);


    /**
     * Manage the lifetime of the libmosquitto library.
     */
    class LibrarySession
    {
    public:
        LibrarySession()
            : result_(mosquitto_lib_init())
        {
        }

        ~LibrarySession()
        {
            if (is_valid())
            {
                mosquitto_lib_cleanup();
            }
        }

        LibrarySession(const LibrarySession&) = delete;
        LibrarySession& operator=(const LibrarySession&) = delete;

        bool is_valid() const noexcept
        {
            return result_ == MOSQ_ERR_SUCCESS;
        }

    private:
        int result_;
    };


    struct PublishState
    {
        std::optional<int> connection_result;
        std::optional<int> publication_result;
        int message_id = 0;
    };


    /**
     * Record the broker's connection acknowledgement.
     *
     * Args:
     *     user_data: Publication state associated with the client.
     *     reason_code: MQTT connection result.
     */
    void on_connect(
        struct mosquitto*,
        void* user_data,
        int reason_code,
        int,
        const mosquitto_property*
    )
    {
        auto& state = *static_cast<PublishState*>(user_data);

        state.connection_result = reason_code;
    }


    /**
     * Record the acknowledgement for the published message.
     *
     * Args:
     *     user_data: Publication state associated with the client.
     *     message_id: Acknowledged message identifier.
     *     reason_code: MQTT publication result.
     */
    void on_publish(
        struct mosquitto*,
        void* user_data,
        int message_id,
        int reason_code,
        const mosquitto_property*
    )
    {
        auto& state = *static_cast<PublishState*>(user_data);

        if (message_id == state.message_id)
        {
            state.publication_result = reason_code;
        }
    }


    /**
     * Check a libmosquitto operation result.
     *
     * Args:
     *     result: Result returned by libmosquitto.
     *     operation: Description of the attempted operation.
     *
     * Returns:
     *     True if the operation succeeded, otherwise false.
     */
    bool check_result(
        int result,
        const char* operation
    )
    {
        if (result == MOSQ_ERR_SUCCESS)
        {
            return true;
        }

        std::cerr << operation
                  << " failed: "
                  << mosquitto_strerror(result)
                  << '\n';

        return false;
    }


    /**
     * Process MQTT network events until an acknowledgement arrives.
     *
     * Args:
     *     client: Active MQTT client handle.
     *     acknowledgement: Result populated by an MQTT callback.
     *
     * Returns:
     *     True if an acknowledgement arrives before the timeout,
     *     otherwise false.
     */
    bool wait_for_acknowledgement(
        struct mosquitto* client,
        const std::optional<int>& acknowledgement
    )
    {
        const auto deadline =
            std::chrono::steady_clock::now() + ACK_TIMEOUT;

        while (!acknowledgement.has_value())
        {
            if (std::chrono::steady_clock::now() >= deadline)
            {
                std::cerr << "MQTT acknowledgement timed out.\n";

                return false;
            }

            if (!check_result(
                mosquitto_loop(client, LOOP_TIMEOUT_MS, 1),
                "MQTT network loop"
            ))
            {
                return false;
            }
        }

        return true;
    }
}


/**
 * Publish a single measurement using authenticated MQTT over TLS.
 *
 * Args:
 *     settings: Broker connection and authentication settings.
 *     topic: MQTT destination topic.
 *     payload: Message content.
 *
 * Returns:
 *     True if the broker acknowledges successful publication,
 *     otherwise false.
 */
bool mqtt::publish(
    const ConnectionSettings& settings,
    const std::string& topic,
    const std::string& payload
)
{
    if (
        settings.host.empty() ||
        settings.username.empty() ||
        settings.password.empty() ||
        settings.ca_file.empty() ||
        settings.port < 1 ||
        settings.port > 65535 ||
        topic.empty() ||
        payload.size() > static_cast<std::size_t>(
            (std::numeric_limits<int>::max)()
        )
    )
    {
        std::cerr << "Invalid MQTT publication settings.\n";

        return false;
    }

    LibrarySession library;

    if (!library.is_valid())
    {
        std::cerr << "Could not initialise libmosquitto.\n";

        return false;
    }

    PublishState state;

    std::unique_ptr<
        struct mosquitto,
        decltype(&mosquitto_destroy)
    > client(
        mosquitto_new(nullptr, true, &state),
        &mosquitto_destroy
    );

    if (!client)
    {
        std::cerr << "Could not create MQTT client.\n";

        return false;
    }

    if (!check_result(
        mosquitto_int_option(
            client.get(),
            MOSQ_OPT_PROTOCOL_VERSION,
            MQTT_PROTOCOL_V5
        ),
        "MQTT protocol configuration"
    ))
    {
        return false;
    }

    if (!check_result(
        mosquitto_username_pw_set(
            client.get(),
            settings.username.c_str(),
            settings.password.c_str()
        ),
        "MQTT authentication configuration"
    ))
    {
        return false;
    }

    if (!check_result(
        mosquitto_tls_set(
            client.get(),
            settings.ca_file.c_str(),
            nullptr,
            nullptr,
            nullptr,
            nullptr
        ),
        "MQTT TLS configuration"
    ))
    {
        return false;
    }

    if (!check_result(
        mosquitto_tls_insecure_set(client.get(), false),
        "MQTT hostname verification configuration"
    ))
    {
        return false;
    }

    mosquitto_connect_v5_callback_set(
        client.get(),
        on_connect
    );

    mosquitto_publish_v5_callback_set(
        client.get(),
        on_publish
    );

    if (!check_result(
        mosquitto_connect(
            client.get(),
            settings.host.c_str(),
            settings.port,
            KEEPALIVE_SECONDS
        ),
        "MQTT connection"
    ))
    {
        return false;
    }

    if (!wait_for_acknowledgement(
        client.get(),
        state.connection_result
    ))
    {
        return false;
    }

    if (state.connection_result.value() != MQTT_RC_SUCCESS)
    {
        std::cerr << "MQTT connection rejected: "
                  << mosquitto_reason_string(
                      state.connection_result.value()
                  )
                  << '\n';

        return false;
    }

    if (!check_result(
        mosquitto_publish(
            client.get(),
            &state.message_id,
            topic.c_str(),
            static_cast<int>(payload.size()),
            payload.data(),
            1,
            false
        ),
        "MQTT publication"
    ))
    {
        return false;
    }

    if (!wait_for_acknowledgement(
        client.get(),
        state.publication_result
    ))
    {
        return false;
    }

    const int reason_code = state.publication_result.value();

    if (
        reason_code != MQTT_RC_SUCCESS &&
        reason_code != MQTT_RC_NO_MATCHING_SUBSCRIBERS
    )
    {
        std::cerr << "MQTT publication rejected: "
                  << mosquitto_reason_string(reason_code)
                  << '\n';

        return false;
    }

    std::cout << "MQTT publication acknowledged: "
              << mosquitto_reason_string(reason_code)
              << '\n';

    check_result(
        mosquitto_disconnect(client.get()),
        "MQTT disconnection"
    );

    return true;
}