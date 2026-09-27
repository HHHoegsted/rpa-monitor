#pragma once

#include <optional>


namespace robot_status
{

/**
 * Determine whether a robot is currently running on the machine.
 *
 * Returns:
 *     True if a robot is running, false if no robot is running, or
 *     std::nullopt if the robot state could not be determined reliably.
 */
std::optional<bool> is_robot_running();

}