#pragma once

#include <string>


namespace mqtt
{
    /**
     * Configuration for an authenticated MQTT connection
     * using TLS and mandatory broker certificate verification.
     */
    struct ConnectionSettings
    {
        std::string host;
        int port = 8883;
        std::string username;
        std::string password;
        std::string ca_file;
    };
}