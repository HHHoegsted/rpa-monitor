#include "machine_report.h"


/**
 * Create a complete diagnostic machine report.
 *
 * Returns:
 *     A complete report containing deterministic diagnostic values.
 */
MachineReport MachineReport::create_diagnostic()
{
    MachineReport report;

    report.hostname_ = diagnostic_hostname_;
    report.cpu_percent_ = 42.0;
    report.ram_percent_ = 42.0;
    report.disk_free_percent_ = 42.0;
    report.robot_running_ = false;
    report.uptime_seconds_ = 1000ULL;

    return report;
}


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
bool MachineReport::set_hostname(const std::string& hostname)
{
    if (hostname.empty())
    {
        return false;
    }

    if (hostname == diagnostic_hostname_)
    {
        return false;
    }

    hostname_ = hostname;

    return true;
}


/**
 * Set the reported CPU usage percentage.
 *
 * Args:
 *     cpu_percent: CPU usage from 0.0 through 100.0.
 *
 * Returns:
 *     True if the value was accepted, otherwise false.
 */
bool MachineReport::set_cpu_percent(double cpu_percent)
{
    if (!is_valid_percentage(cpu_percent))
    {
        return false;
    }

    cpu_percent_ = cpu_percent;

    return true;
}


/**
 * Set the reported RAM usage percentage.
 *
 * Args:
 *     ram_percent: RAM usage from 0.0 through 100.0.
 *
 * Returns:
 *     True if the value was accepted, otherwise false.
 */
bool MachineReport::set_ram_percent(double ram_percent)
{
    if (!is_valid_percentage(ram_percent))
    {
        return false;
    }

    ram_percent_ = ram_percent;

    return true;
}


/**
 * Set the reported free disk percentage.
 *
 * Args:
 *     disk_free_percent: Free disk space from 0.0 through 100.0.
 *
 * Returns:
 *     True if the value was accepted, otherwise false.
 */
bool MachineReport::set_disk_free_percent(double disk_free_percent)
{
    if (!is_valid_percentage(disk_free_percent))
    {
        return false;
    }

    disk_free_percent_ = disk_free_percent;

    return true;
}


/**
 * Set whether a robot is currently running.
 *
 * Args:
 *     robot_running: True when a robot is currently running.
 */
void MachineReport::set_robot_running(bool robot_running)
{
    robot_running_ = robot_running;
}


/**
 * Set the reported system uptime.
 *
 * Args:
 *     uptime_seconds: Time elapsed since Windows started, in seconds.
 */
void MachineReport::set_uptime_seconds(
    unsigned long long uptime_seconds
)
{
    uptime_seconds_ = uptime_seconds;
}


/**
 * Check whether all required report values have been provided.
 *
 * Returns:
 *     True when the report contains all required values.
 */
bool MachineReport::is_complete() const
{
    return hostname_.has_value()
        && cpu_percent_.has_value()
        && ram_percent_.has_value()
        && disk_free_percent_.has_value()
        && robot_running_.has_value()
        && uptime_seconds_.has_value();
}


/**
 * Check whether this is the reserved diagnostic report.
 *
 * Returns:
 *     True when the report represents DEEP-THOUGHT.
 */
bool MachineReport::is_diagnostic() const
{
    return hostname_.has_value()
        && hostname_.value() == diagnostic_hostname_;
}


/**
 * Check whether a percentage value is within the allowed range.
 *
 * Args:
 *     value: Percentage value to validate.
 *
 * Returns:
 *     True when the value is between 0.0 and 100.0 inclusive.
 */
bool MachineReport::is_valid_percentage(double value)
{
    return value >= 0.0 && value <= 100.0;
}