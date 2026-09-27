#include "system_metrics.h"

#include <chrono>
#include <thread>
#include <windows.h>


namespace
{

/**
 * Convert a Windows FILETIME value to a 64-bit integer.
 *
 * Args:
 *     file_time: Windows FILETIME value to convert.
 *
 * Returns:
 *     The FILETIME value represented as an unsigned 64-bit integer.
 */
unsigned long long file_time_to_integer(const FILETIME& file_time)
{
    ULARGE_INTEGER value{};

    value.LowPart = file_time.dwLowDateTime;
    value.HighPart = file_time.dwHighDateTime;

    return value.QuadPart;
}

}


namespace system_metrics
{

/**
 * Get the current CPU usage percentage.
 *
 * Returns:
 *     The CPU usage percentage from 0.0 through 100.0, or std::nullopt
 *     if the measurement could not be obtained.
 */
std::optional<double> get_cpu_percent()
{
    FILETIME idle_time_start{};
    FILETIME kernel_time_start{};
    FILETIME user_time_start{};

    if (
        !GetSystemTimes(
            &idle_time_start,
            &kernel_time_start,
            &user_time_start
        )
    )
    {
        return std::nullopt;
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(250)
    );

    FILETIME idle_time_end{};
    FILETIME kernel_time_end{};
    FILETIME user_time_end{};

    if (
        !GetSystemTimes(
            &idle_time_end,
            &kernel_time_end,
            &user_time_end
        )
    )
    {
        return std::nullopt;
    }

    unsigned long long idle_delta =
        file_time_to_integer(idle_time_end)
        - file_time_to_integer(idle_time_start);

    unsigned long long kernel_delta =
        file_time_to_integer(kernel_time_end)
        - file_time_to_integer(kernel_time_start);

    unsigned long long user_delta =
        file_time_to_integer(user_time_end)
        - file_time_to_integer(user_time_start);

    unsigned long long total_delta =
        kernel_delta + user_delta;

    if (total_delta == 0)
    {
        return std::nullopt;
    }

    double cpu_percent =
        100.0
        * (
            1.0
            - static_cast<double>(idle_delta)
                / static_cast<double>(total_delta)
        );

    return cpu_percent;
}

/**
 * Get the current RAM usage percentage.
 *
 * Returns:
 *     The RAM usage percentage from 0.0 through 100.0, or std::nullopt
 *     if the measurement could not be obtained.
 */
std::optional<double> get_ram_percent()
{
    MEMORYSTATUSEX memory_status{};

    memory_status.dwLength = sizeof(memory_status);

    if (!GlobalMemoryStatusEx(&memory_status))
    {
        return std::nullopt;
    }

    return static_cast<double>(memory_status.dwMemoryLoad);
}

/**
 * Get the current free disk space percentage for the system drive.
 *
 * Returns:
 *     The free disk space percentage from 0.0 through 100.0, or std::nullopt
 *     if the measurement could not be obtained.
 */
std::optional<double> get_disk_free_percent()
{
    ULARGE_INTEGER free_bytes_available{};
    ULARGE_INTEGER total_bytes{};
    ULARGE_INTEGER total_free_bytes{};

    if (
        !GetDiskFreeSpaceExW(
            L"C:\\",
            &free_bytes_available,
            &total_bytes,
            &total_free_bytes
        )
    )
    {
        return std::nullopt;
    }

    if (total_bytes.QuadPart == 0)
    {
        return std::nullopt;
    }

    double free_percent =
        100.0
        * static_cast<double>(total_free_bytes.QuadPart)
        / static_cast<double>(total_bytes.QuadPart);

    return free_percent;
}

}