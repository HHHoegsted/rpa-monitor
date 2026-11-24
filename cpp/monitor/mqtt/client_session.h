#pragma once

#include <memory>
#include <mosquitto.h>

#include "connection_settings.h"


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
    );


    /**
     * Own and configure an MQTT client handle through RAII.
     *
     * Requires an active LibrarySession throughout its lifetime.
     */
    class ClientSession
    {
    public:
        /**
         * Create and configure an authenticated MQTT client
         * with mandatory TLS certificate verification.
         *
         * Args:
         *     settings: Broker connection settings.
         *     client_id: MQTT client identifier, or nullptr
         *         for an automatically generated identifier.
         *     clean_session: Whether to use a clean session.
         *     user_data: Application data passed to callbacks.
         */
        ClientSession(
            const ConnectionSettings& settings,
            const char* client_id,
            bool clean_session,
            void* user_data
        );

        ~ClientSession() = default;

        ClientSession(const ClientSession&) = delete;
        ClientSession& operator=(const ClientSession&) = delete;

        ClientSession(ClientSession&&) = delete;
        ClientSession& operator=(ClientSession&&) = delete;

        /**
         * Determine whether client creation and configuration
         * succeeded.
         *
         * Returns:
         *     True if the client is available, otherwise false.
         */
        bool is_valid() const noexcept;

        /**
         * Retrieve the underlying MQTT client handle.
         *
         * Returns:
         *     A borrowed client handle, or nullptr if invalid.
         */
        struct mosquitto* get() const noexcept;

    private:
        std::unique_ptr<
            struct mosquitto,
            decltype(&mosquitto_destroy)
        > client_;
    };
}