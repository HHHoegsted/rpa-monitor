#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include <pqxx/pqxx>

#include "mqtt/subscriber.h"
#include "mqtt/report_parser.h"
#include "database/report_repository.h"


/**
 * Retrieve an environment variable.
 *
 * Args:
 *     name: Name of the environment variable.
 *
 * Returns:
 *     The configured value, or an empty string if unavailable.
 */
std::string get_environment_variable(const char* name)
{
    const char* value = std::getenv(name);

    return value != nullptr ? std::string(value) : "";
}


/**
 * Load MQTT collector connection settings.
 *
 * Returns:
 *     MQTT connection and authentication settings.
 */
mqtt::ConnectionSettings load_connection_settings()
{
    mqtt::ConnectionSettings settings;

    settings.host =
        get_environment_variable("MQTT_HOST");

    settings.username =
        get_environment_variable("MQTT_COLLECTOR_USERNAME");

    settings.password =
        get_environment_variable("MQTT_COLLECTOR_PASSWORD");

    settings.ca_file =
        get_environment_variable("MQTT_CA_FILE");

    settings.port = 8883;

    return settings;
}


/**
 * Verify that the PostgreSQL database is accessible.
 *
 * Returns:
 *     True if the connection and test query succeed,
 *     otherwise false.
 */
bool verify_database_connection()
{
    try
    {
        pqxx::connection connection;
        pqxx::work transaction{connection};

        const auto result = transaction.exec("SELECT 1");

        transaction.commit();

        return result.size() == 1
            && result[0][0].as<int>() == 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Database connection failed: "
                  << error.what()
                  << '\n';

        return false;
    }
}


/**
 * Validate and persist a received MQTT machine report.
 *
 * Args:
 *     topic: MQTT topic on which the report was received.
 *     payload: JSON measurement payload.
 */
void handle_measurement(
    const std::string& topic,
    const std::string& payload
)
{
    const auto report = mqtt::parse_machine_report(
        topic,
        payload
    );

    if (!report.has_value())
    {
        std::cerr << "Rejected invalid machine report.\n";

        return;
    }

    if (!save_machine_report(report.value()))
    {
        std::cerr << "Could not persist report from: "
                  << report->hostname()
                  << '\n';

        return;
    }

    std::cout << "Saved machine report: "
              << report->hostname()
              << '\n';
}


/**
 * Verify database connectivity and start the MQTT collector.
 *
 * Returns:
 *     0 on normal termination, otherwise 1.
 */
int main()
{
    std::cout << std::unitbuf;

    if (!verify_database_connection())
    {
        return 1;
    }

    const auto settings = load_connection_settings();

    return mqtt::subscribe(
        settings,
        "rpa-drift-collector",
        "rpa/machines/+/measurements",
        handle_measurement
    ) ? 0 : 1;
}