#pragma once

#include <optional>
#include <string>


/**
 * Represents a machine health report collected from one client.
 */
class MachineReport
{
public:
    /**
     * Create a complete diagnostic machine report.
     *
     * Returns:
     *     A complete report containing deterministic diagnostic values.
     */
    static MachineReport create_diagnostic();

    /**
     * Set the hostname reported by the client.
     *
     * The reserved diagnostic hostname DEEP-THOUGHT cannot be assigned
     * through this method.
     *
     * Args:
     *     hostname: Hostname of the reporting machine.
     *
     * Returns:
     *     True if the hostname was accepted, otherwise false.
     */
    bool set_hostname(const std::string& hostname);

    /**
     * Set the reported CPU usage percentage.
     *
     * Args:
     *     cpu_percent: CPU usage from 0.0 through 100.0.
     *
     * Returns:
     *     True if the value was accepted, otherwise false.
     */
    bool set_cpu_percent(double cpu_percent);

    /**
     * Set the reported RAM usage percentage.
     *
     * Args:
     *     ram_percent: RAM usage from 0.0 through 100.0.
     *
     * Returns:
     *     True if the value was accepted, otherwise false.
     */
    bool set_ram_percent(double ram_percent);

    /**
     * Set the reported free disk percentage.
     *
     * Args:
     *     disk_free_percent: Free disk space from 0.0 through 100.0.
     *
     * Returns:
     *     True if the value was accepted, otherwise false.
     */
    bool set_disk_free_percent(double disk_free_percent);

    /**
     * Set whether a robot is currently running.
     *
     * Args:
     *     robot_running: True when a robot is currently running.
     */
    void set_robot_running(bool robot_running);

    /**
     * Check whether all required report values have been provided.
     *
     * Returns:
     *     True when the report contains all required values.
     */
    bool is_complete() const;

    /**
     * Check whether this is the reserved diagnostic report.
     *
     * Returns:
     *     True when the report represents DEEP-THOUGHT.
     */
    bool is_diagnostic() const;

private:
    /**
     * Check whether a percentage value is within the allowed range.
     *
     * Args:
     *     value: Percentage value to validate.
     *
     * Returns:
     *     True when the value is between 0.0 and 100.0 inclusive.
     */
    static bool is_valid_percentage(double value);

    static constexpr const char* diagnostic_hostname_ = "DEEP-THOUGHT";

    std::optional<std::string> hostname_;
    std::optional<double> cpu_percent_;
    std::optional<double> ram_percent_;
    std::optional<double> disk_free_percent_;
    std::optional<bool> robot_running_;
};