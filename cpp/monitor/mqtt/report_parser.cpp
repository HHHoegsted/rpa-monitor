#include "report_parser.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <limits>


namespace mqtt
{
    /**
     * Parse an MQTT measurement payload and validate its contents
     * against the machine report model and originating topic.
     *
     * Args:
     *     topic: MQTT topic on which the message was received.
     *     payload: JSON measurement payload.
     *
     * Returns:
     *     A complete, validated MachineReport, or no value
     *     if validation fails.
     */
    std::optional<MachineReport> parse_machine_report(
        const std::string& topic,
        const std::string& payload
    )
    {
        constexpr std::size_t MAX_PAYLOAD_SIZE = 4096;

        if (payload.empty() || payload.size() > MAX_PAYLOAD_SIZE)
        {
            return std::nullopt;
        }

        try
        {
            const auto data = nlohmann::json::parse(payload);

            if (!data.is_object())
            {
                return std::nullopt;
            }

            const auto& hostname = data.at("hostname");
            const auto& cpu = data.at("cpu_percent");
            const auto& ram = data.at("ram_percent");
            const auto& disk = data.at("disk_free_percent");
            const auto& robot = data.at("robot_running");
            const auto& uptime = data.at("uptime_seconds");

            if (
                !hostname.is_string() ||
                !cpu.is_number() ||
                !ram.is_number() ||
                !disk.is_number() ||
                !robot.is_boolean() ||
                !uptime.is_number_integer()
            )
            {
                return std::nullopt;
            }

            const auto hostname_value =
                hostname.get<std::string>();

            if (
                hostname_value.empty() ||
                hostname_value.find_first_not_of(
                    "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "0123456789-_."
                ) != std::string::npos
            )
            {
                return std::nullopt;
            }

            const std::string expected_topic =
                "rpa/machines/"
                + hostname_value
                + "/measurements";

            if (topic != expected_topic)
            {
                return std::nullopt;
            }

            std::uint64_t uptime_value;

            if (uptime.is_number_unsigned())
            {
                uptime_value = uptime.get<std::uint64_t>();
            }
            else
            {
                const auto signed_uptime =
                    uptime.get<std::int64_t>();

                if (signed_uptime < 0)
                {
                    return std::nullopt;
                }

                uptime_value =
                    static_cast<std::uint64_t>(signed_uptime);
            }

            if (
                uptime_value > static_cast<std::uint64_t>(
                    (std::numeric_limits<std::int64_t>::max)()
                )
            )
            {
                return std::nullopt;
            }

            MachineReport report;

            if (
                !report.set_hostname(hostname_value) ||
                !report.set_cpu_percent(cpu.get<double>()) ||
                !report.set_ram_percent(ram.get<double>()) ||
                !report.set_disk_free_percent(disk.get<double>())
            )
            {
                return std::nullopt;
            }

            report.set_robot_running(robot.get<bool>());
            report.set_uptime_seconds(uptime_value);

            if (!report.is_complete())
            {
                return std::nullopt;
            }

            return report;
        }
        catch (const nlohmann::json::exception&)
        {
            return std::nullopt;
        }
    }
}