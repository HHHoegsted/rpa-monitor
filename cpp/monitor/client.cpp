#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>
#include <string>
#include <optional>
#include <sstream>
#include <locale>
#include <iomanip>

#include "metrics/system_metrics.h"
#include "metrics/robot_status.h"
#include "mqtt/publisher.h"


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
    const DWORD required_size =
        GetEnvironmentVariableA(name, nullptr, 0);

    if (required_size == 0)
    {
        return "";
    }

    std::string value(required_size, '\0');

    const DWORD copied_size = GetEnvironmentVariableA(
        name,
        value.data(),
        required_size
    );

    if (copied_size == 0 || copied_size >= required_size)
    {
        return "";
    }

    value.resize(copied_size);

    return value;
}


/**
 * Retrieve the DNS hostname of the local machine.
 *
 * Returns:
 *     The hostname, or an empty string if retrieval fails.
 */
std::string get_hostname()
{
    char hostname[256]{};
    DWORD size = sizeof(hostname);

    if (!GetComputerNameExA(
        ComputerNameDnsHostname,
        hostname,
        &size
    ))
    {
        std::cerr << "Could not retrieve hostname.\n";

        return "";
    }

    return std::string(hostname, size);
}


/**
 * Load MQTT connection settings from environment variables.
 *
 * Returns:
 *     MQTT connection settings.
 */
mqtt::ConnectionSettings load_connection_settings()
{
    mqtt::ConnectionSettings settings;

    settings.host =
        get_environment_variable("MQTT_HOST");

    settings.username =
        get_environment_variable("MQTT_PUBLISHER_USERNAME");

    settings.password =
        get_environment_variable("MQTT_PUBLISHER_PASSWORD");

    settings.ca_file =
        get_environment_variable("MQTT_CA_FILE");

    settings.port = 8883;

    return settings;
}


/**
 * Collect machine measurements and construct a JSON payload.
 *
 * Args:
 *     hostname: Hostname of the monitored machine.
 *
 * Returns:
 *     A JSON measurement payload, or no value if collection fails.
 */
std::optional<std::string> collect_measurements(
    const std::string& hostname
)
{
    if (
        hostname.empty() ||
        hostname.find_first_not_of(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789-_."
        ) != std::string::npos
    )
    {
        std::cerr << "Invalid machine hostname.\n";

        return std::nullopt;
    }

    const auto cpu = system_metrics::get_cpu_percent();
    const auto ram = system_metrics::get_ram_percent();
    const auto disk = system_metrics::get_disk_free_percent();
    const auto robot = robot_status::is_robot_running();
    const auto uptime = system_metrics::get_uptime_seconds();

    if (
        !cpu.has_value() ||
        !ram.has_value() ||
        !disk.has_value() ||
        !robot.has_value()
    )
    {
        std::cerr << "Could not collect machine measurements.\n";

        return std::nullopt;
    }

    std::ostringstream payload;

    payload.imbue(std::locale::classic());

    payload << std::fixed
            << std::setprecision(6)
            << std::boolalpha
            << "{"
            << "\"hostname\":\"" << hostname << "\","
            << "\"cpu_percent\":" << cpu.value() << ","
            << "\"ram_percent\":" << ram.value() << ","
            << "\"disk_free_percent\":" << disk.value() << ","
            << "\"robot_running\":" << robot.value() << ","
            << "\"uptime_seconds\":" << uptime
            << "}";

    return payload.str();
}


/**
 * Collect and publish a single machine report.
 *
 * Returns:
 *     0 on successful publication, otherwise 1.
 */
int main()
{
    const auto settings = load_connection_settings();

    const std::string hostname = get_hostname();

    if (hostname.empty())
    {
        return 1;
    }

    const auto payload = collect_measurements(hostname);

    if (!payload.has_value())
    {
        return 1;
    }

    const std::string topic =
        "rpa/machines/" + hostname + "/measurements";

    if (!mqtt::publish(settings, topic, payload.value()))
    {
        return 1;
    }

    return 0;
}