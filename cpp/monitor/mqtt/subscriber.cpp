#include "subscriber.h"
#include "library_session.h"
#include "client_session.h"

#include <mqtt_protocol.h>

#include <exception>
#include <iostream>
#include <memory>


namespace
{
    constexpr int KEEPALIVE_SECONDS = 30;
    constexpr int SESSION_EXPIRY_SECONDS = 3600;
    constexpr int LOOP_TIMEOUT_MS = 1000;
    constexpr int MAX_PAYLOAD_SIZE = 4096;


    struct SubscriberState
    {
        const std::string& topic_filter;
        const mqtt::MessageHandler& handler;
        bool failed = false;
    };


    /**
     * Establish the subscription following successful authentication.
     *
     * Args:
     *     client: MQTT client handle.
     *     user_data: Subscriber state.
     *     reason_code: Broker connection acknowledgement.
     */
    void on_connect(
        struct mosquitto* client,
        void* user_data,
        int reason_code,
        int,
        const mosquitto_property*
    )
    {
        auto& state =
            *static_cast<SubscriberState*>(user_data);

        if (reason_code != MQTT_RC_SUCCESS)
        {
            std::cerr << "MQTT connection rejected: "
                      << mosquitto_reason_string(reason_code)
                      << '\n';

            state.failed = true;
            mosquitto_disconnect(client);

            return;
        }

        std::cout << "Connected to MQTT broker.\n";

        if (!mqtt::check_result(
            mosquitto_subscribe_v5(
                client,
                nullptr,
                state.topic_filter.c_str(),
                1,
                MQTT_SUB_OPT_SEND_RETAIN_NEVER,
                nullptr
            ),
            "MQTT subscription"
        ))
        {
            state.failed = true;
            mosquitto_disconnect(client);
        }
    }


    /**
     * Verify that the broker granted the requested subscription.
     *
     * Args:
     *     client: MQTT client handle.
     *     user_data: Subscriber state.
     *     qos_count: Number of subscription acknowledgement codes.
     *     granted_qos: Subscription acknowledgement codes.
     */
    void on_subscribe(
        struct mosquitto* client,
        void* user_data,
        int,
        int qos_count,
        const int* granted_qos,
        const mosquitto_property*
    )
    {
        auto& state =
            *static_cast<SubscriberState*>(user_data);

        if (
            qos_count != 1 ||
            granted_qos == nullptr ||
            granted_qos[0] != MQTT_RC_GRANTED_QOS1
        )
        {
            std::cerr << "MQTT subscription rejected or downgraded.\n";

            state.failed = true;
            mosquitto_disconnect(client);

            return;
        }

        std::cout << "Subscribed to: "
                  << state.topic_filter
                  << '\n';
    }


    /**
     * Deliver a received MQTT message to the application handler.
     *
     * Args:
     *     user_data: Subscriber state.
     *     message: Message received from the broker.
     */
    void on_message(
        struct mosquitto*,
        void* user_data,
        const struct mosquitto_message* message,
        const mosquitto_property*
    )
    {
        auto& state =
            *static_cast<SubscriberState*>(user_data);

        if (
            message == nullptr ||
            message->topic == nullptr ||
            message->payload == nullptr ||
            message->payloadlen <= 0 ||
            message->payloadlen > MAX_PAYLOAD_SIZE
        )
        {
            std::cerr << "Rejected invalid MQTT message.\n";

            return;
        }

        try
        {
            const std::string topic(message->topic);

            const std::string payload(
                static_cast<const char*>(message->payload),
                static_cast<std::size_t>(message->payloadlen)
            );

            state.handler(topic, payload);
        }
        catch (const std::exception& error)
        {
            std::cerr << "MQTT message handling failed: "
                      << error.what()
                      << '\n';
        }
        catch (...)
        {
            std::cerr << "Unknown MQTT message handling failure.\n";
        }
    }
}


/**
 * Connect to an authenticated MQTT broker and continuously
 * deliver received messages to the application handler.
 *
 * Args:
 *     settings: Broker connection and authentication settings.
 *     client_id: Stable subscriber identifier.
 *     topic_filter: MQTT topic filter.
 *     handler: Application callback for incoming messages.
 *
 * Returns:
 *     True on normal termination, otherwise false.
 */
bool mqtt::subscribe(
    const ConnectionSettings& settings,
    const std::string& client_id,
    const std::string& topic_filter,
    const MessageHandler& handler
)
{
    if (
        client_id.empty() ||
        topic_filter.empty() ||
        !handler
    )
    {
        std::cerr << "Invalid MQTT subscription settings.\n";

        return false;
    }

    LibrarySession library;

    if (!library.is_valid())
    {
        std::cerr << "Could not initialise libmosquitto.\n";

        return false;
    }

    SubscriberState state{
        topic_filter,
        handler
    };

    ClientSession client(
        settings,
        client_id.c_str(),
        false,
        &state
    );

    if (!client.is_valid())
    {
        return false;
    }

    mosquitto_connect_v5_callback_set(
        client.get(),
        on_connect
    );

    mosquitto_subscribe_v5_callback_set(
        client.get(),
        on_subscribe
    );

    mosquitto_message_v5_callback_set(
        client.get(),
        on_message
    );

    if (!check_result(
        mosquitto_reconnect_delay_set(
            client.get(),
            1,
            30,
            true
        ),
        "MQTT reconnect configuration"
    ))
    {
        return false;
    }

    mosquitto_property* raw_properties = nullptr;

    const int property_result =
        mosquitto_property_add_int32(
            &raw_properties,
            MQTT_PROP_SESSION_EXPIRY_INTERVAL,
            SESSION_EXPIRY_SECONDS
        );

    const auto property_deleter =
        [](mosquitto_property* properties)
        {
            mosquitto_property_free_all(&properties);
        };

    std::unique_ptr<
        mosquitto_property,
        decltype(property_deleter)
    > properties(
        raw_properties,
        property_deleter
    );

    if (!check_result(
        property_result,
        "MQTT session configuration"
    ))
    {
        return false;
    }

    if (!check_result(
        mosquitto_connect_bind_v5(
            client.get(),
            settings.host.c_str(),
            settings.port,
            KEEPALIVE_SECONDS,
            nullptr,
            properties.get()
        ),
        "MQTT connection"
    ))
    {
        return false;
    }

    const int loop_result = mosquitto_loop_forever(
        client.get(),
        LOOP_TIMEOUT_MS,
        1
    );

    return check_result(
        loop_result,
        "MQTT subscriber loop"
    ) && !state.failed;
}