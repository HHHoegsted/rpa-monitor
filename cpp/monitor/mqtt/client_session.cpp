#include "client_session.h"

#include <mqtt_protocol.h>
#include <iostream>


namespace mqtt
{
    /**
     * Check the result of a libmosquitto operation.
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
     * Create and configure an MQTT client with authentication,
     * TLS encryption and mandatory certificate verification.
     *
     * Args:
     *     settings: Broker connection settings.
     *     client_id: MQTT client identifier.
     *     clean_session: Whether to use a clean session.
     *     user_data: Application data passed to callbacks.
     */
    ClientSession::ClientSession(
        const ConnectionSettings& settings,
        const char* client_id,
        bool clean_session,
        void* user_data
    )
        : client_(nullptr, &mosquitto_destroy)
    {
        if (
            settings.host.empty() ||
            settings.username.empty() ||
            settings.password.empty() ||
            settings.ca_file.empty() ||
            settings.port < 1 ||
            settings.port > 65535
        )
        {
            std::cerr << "Invalid MQTT connection settings.\n";

            return;
        }

        client_.reset(
            mosquitto_new(
                client_id,
                clean_session,
                user_data
            )
        );

        if (!client_)
        {
            std::cerr << "Could not create MQTT client.\n";

            return;
        }

        if (
            !check_result(
                mosquitto_int_option(
                    client_.get(),
                    MOSQ_OPT_PROTOCOL_VERSION,
                    MQTT_PROTOCOL_V5
                ),
                "MQTT protocol configuration"
            ) ||
            !check_result(
                mosquitto_username_pw_set(
                    client_.get(),
                    settings.username.c_str(),
                    settings.password.c_str()
                ),
                "MQTT authentication configuration"
            ) ||
            !check_result(
                mosquitto_tls_set(
                    client_.get(),
                    settings.ca_file.c_str(),
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                ),
                "MQTT TLS configuration"
            ) ||
            !check_result(
                mosquitto_tls_insecure_set(
                    client_.get(),
                    false
                ),
                "MQTT hostname verification configuration"
            )
        )
        {
            client_.reset();

            return;
        }
    }


    /**
     * Determine whether the MQTT client was successfully
     * created and configured.
     *
     * Returns:
     *     True if the client is available, otherwise false.
     */
    bool ClientSession::is_valid() const noexcept
    {
        return client_ != nullptr;
    }


    /**
     * Retrieve a borrowed MQTT client handle.
     *
     * Returns:
     *     The underlying client handle, or nullptr if invalid.
     */
    struct mosquitto* ClientSession::get() const noexcept
    {
        return client_.get();
    }
}