#pragma once

#include <functional>
#include <string>

#include "connection_settings.h"


namespace mqtt
{
    using MessageHandler = std::function<void(
        const std::string& topic,
        const std::string& payload
    )>;


    /**
     * Connect to an MQTT broker and continuously receive messages.
     *
     * Args:
     *     settings: Broker connection and authentication settings.
     *     client_id: Stable identifier for the subscriber.
     *     topic_filter: MQTT topic filter to subscribe to.
     *     handler: Callback invoked for each received message.
     *
     * Returns:
     *     True on normal termination, otherwise false.
     */
    bool subscribe(
        const ConnectionSettings& settings,
        const std::string& client_id,
        const std::string& topic_filter,
        const MessageHandler& handler
    );
}