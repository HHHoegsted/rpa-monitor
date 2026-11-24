#pragma once

#include <string>

#include "connection_settings.h"


namespace mqtt
{
    /**
     * Publish a single message over an authenticated TLS connection.
     *
     * Establishes a connection, publishes using QoS 1,
     * receives the broker acknowledgement and disconnects.
     *
     * Args:
     *     settings: Broker connection and authentication settings.
     *     topic: MQTT topic to publish to.
     *     payload: Message content to publish.
     *
     * Returns:
     *     True if the broker acknowledges successful publication,
     *     otherwise false.
     */
    bool publish(
        const ConnectionSettings& settings,
        const std::string& topic,
        const std::string& payload
    );
}