#pragma once

#include <optional>


namespace system_metrics
{

/**
 * Get the current CPU usage percentage.
 *
 * Returns:
 *     The CPU usage percentage from 0.0 through 100.0, or std::nullopt
 *     if the measurement could not be obtained.
 */
std::optional<double> get_cpu_percent();

/**
 * Get the current RAM usage percentage.
 *
 * Returns:
 *     The RAM usage percentage from 0.0 through 100.0, or std::nullopt
 *     if the measurement could not be obtained.
 */
std::optional<double> get_ram_percent();

/**
 * Get the current free disk space percentage.
 *
 * Returns:
 *     The free disk space percentage from 0.0 through 100.0, or std::nullopt
 *     if the measurement could not be obtained.
 */
std::optional<double> get_disk_free_percent();

/**
 * Get the time elapsed since Windows started.
 *
 * Returns:
 *     System uptime in whole seconds.
 */
unsigned long long get_uptime_seconds();

}