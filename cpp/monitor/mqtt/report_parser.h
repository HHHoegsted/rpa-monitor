#pragma once

#include <optional>
#include <string>

#include "../models/machine_report.h"


namespace mqtt
{
    /**
     * Parse and validate an incoming MQTT measurement.
     *
     * Args:
     *     topic: MQTT topic on which the message was received.
     *     payload: JSON measurement payload.
     *
     * Returns:
     *     A complete, validated MachineReport, or no value
     *     if the message is invalid.
     */
    std::optional<MachineReport> parse_machine_report(
        const std::string& topic,
        const std::string& payload
    );
}