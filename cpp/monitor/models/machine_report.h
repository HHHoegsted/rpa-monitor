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
     * Set the hostname reported by the client.
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
     *     True if accepted, otherwise false.
     */
    bool set_cpu_percent(double cpu_percent);

    /**
     * Set the reported RAM usage percentage.
     *
     * Args:
     *     ram_percent: RAM usage from 0.0 through 100.0.
     *
     * Returns:
     *     True if accepted, otherwise false.
     */
    bool set_ram_percent(double ram_percent);

    /**
     * Set the reported free disk percentage.
     *
     * Args:
     *     disk_free_percent: Free disk space from 0.0 through 100.0.
     *
     * Returns:
     *     True if accepted, otherwise false.
     */
    bool set_disk_free_percent(double disk_free_percent);

    /**
     * Set whether a robot is currently running.
     *
     * Args:
     *     robot_running: True when a robot is running.
     */
    void set_robot_running(bool robot_running);

    /**
     * Set the reported system uptime.
     *
     * Args:
     *     uptime_seconds: System uptime in seconds.
     */
    void set_uptime_seconds(unsigned long long uptime_seconds);

    /**
     * Get the hostname of the reporting machine.
     *
     * Requires:
     *     The report must be complete.
     *
     * Returns:
     *     A constant reference to the hostname.
     */
    const std::string& hostname() const;

    /**
     * Get the CPU usage percentage.
     *
     * Requires:
     *     The report must be complete.
     *
     * Returns:
     *     CPU usage percentage.
     */
    double cpu_percent() const;

    /**
     * Get the RAM usage percentage.
     *
     * Requires:
     *     The report must be complete.
     *
     * Returns:
     *     RAM usage percentage.
     */
    double ram_percent() const;

    /**
     * Get the free disk space percentage.
     *
     * Requires:
     *     The report must be complete.
     *
     * Returns:
     *     Free disk space percentage.
     */
    double disk_free_percent() const;

    /**
     * Get whether a robot is running.
     *
     * Requires:
     *     The report must be complete.
     *
     * Returns:
     *     True if a robot is running, otherwise false.
     */
    bool robot_running() const;

    /**
     * Get the system uptime.
     *
     * Requires:
     *     The report must be complete.
     *
     * Returns:
     *     System uptime in seconds.
     */
    unsigned long long uptime_seconds() const;

    /**
     * Check whether all required values have been provided.
     *
     * Returns:
     *     True when the report contains all required values.
     */
    bool is_complete() const;

private:
    /**
     * Check whether a percentage is within the allowed range.
     *
     * Args:
     *     value: Percentage value to validate.
     *
     * Returns:
     *     True when between 0.0 and 100.0 inclusive.
     */
    static bool is_valid_percentage(double value);

    std::optional<std::string> hostname_;
    std::optional<double> cpu_percent_;
    std::optional<double> ram_percent_;
    std::optional<double> disk_free_percent_;
    std::optional<bool> robot_running_;
    std::optional<unsigned long long> uptime_seconds_;
};