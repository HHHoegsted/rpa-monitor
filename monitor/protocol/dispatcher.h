#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "../models/machine_report.h"
#include "protocol_state.h"
#include "message.h"


/**
 * Validates protocol messages and routes them to their command handlers.
 */
class ProtocolDispatcher
{
public:
    /**
     * Create a dispatcher with the supported protocol commands.
     */
    ProtocolDispatcher();

    /**
     * Validate and dispatch a protocol message for the current conversation state.
     *
     * Args:
     *     message: Parsed protocol message to handle.
     *     report: Machine report belonging to the current client conversation.
     *     state: Current protocol state belonging to the client conversation.
     *
     * Returns:
     *     The protocol response produced by validation or the command handler.
     */
    std::string dispatch(
        const ProtocolMessage& message,
        MachineReport& report,
        ProtocolState& state
    ) const;

private:
    using Handler = std::function<
        std::string(const ProtocolMessage&, MachineReport&)
    >;

    /**
     * Handle a HELLO command.
     *
     * Args:
     *     message: Parsed HELLO protocol message.
     *     report: Machine report belonging to the current client conversation.
     *
     * Returns:
     *     Protocol response for the HELLO command.
     */
    static std::string handle_hello(
        const ProtocolMessage& message,
        MachineReport& report
    );

    /**
     * Handle a CPU command.
     *
     * Args:
     *     message: Parsed CPU protocol message.
     *     report: Machine report belonging to the current client conversation.
     *
     * Returns:
     *     Protocol response for the CPU command.
     */
    static std::string handle_cpu(
        const ProtocolMessage& message,
        MachineReport& report
    );

    /**
     * Handle a RAM command.
     *
     * Args:
     *     message: Parsed RAM protocol message.
     *     report: Machine report belonging to the current client conversation.
     *
     * Returns:
     *     Protocol response for the RAM command.
     */
    static std::string handle_ram(
        const ProtocolMessage& message,
        MachineReport& report
    );

    /**
     * Handle a DISK_FREE command.
     *
     * Args:
     *     message: Parsed DISK_FREE protocol message.
     *     report: Machine report belonging to the current client conversation.
     *
     * Returns:
     *     Protocol response for the DISK_FREE command.
     */
    static std::string handle_disk_free(
        const ProtocolMessage& message,
        MachineReport& report
    );

    /**
     * Handle a ROBOT_RUNNING protocol message.
     *
     * Args:
     *     message: Parsed protocol message containing the robot-running value.
     *     report: Machine report to update.
     *
     * Returns:
     *     Protocol response describing whether the value was accepted.
     */
    static std::string handle_robot_running(
        const ProtocolMessage& message,
        MachineReport& report
    );

    /**
     * Handle a DONE command.
     *
     * Args:
     *     message: Parsed DONE protocol message.
     *     report: Machine report belonging to the current client conversation.
     *
     * Returns:
     *     Protocol response for the DONE command.
     */
    static std::string handle_done(
        const ProtocolMessage& message,
        MachineReport& report
    );

    /**
     * Handle a BANANA command.
     *
     * Args:
     *     message: Parsed BANANA protocol message.
     *     report: Machine report belonging to the current client conversation.
     *
     * Returns:
     *     Protocol response for the BANANA command.
     */
    static std::string handle_banana(
        const ProtocolMessage& message,
        MachineReport& report
    );

    std::unordered_map<std::string, Handler> handlers_;
};