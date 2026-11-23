#include "protocol_state_machine.h"


/**
 * Determine the next protocol state for a command.
 *
 * Args:
 *     current_state: Current state of the client conversation.
 *     command: Protocol command received from the client.
 *
 * Returns:
 *     The next protocol state when the command is valid for the current
 *     state, otherwise no value.
 */
std::optional<ProtocolState> ProtocolStateMachine::next_state(
    ProtocolState current_state,
    const std::string& command
)
{
    switch (current_state)
    {
        case ProtocolState::WAITING_FOR_INITIAL_MESSAGE:
            if (command == "HELLO")
            {
                return ProtocolState::WAITING_FOR_CPU;
            }

            if (command == "BANANA")
            {
                return ProtocolState::COMPLETE;
            }

            return std::nullopt;

        case ProtocolState::WAITING_FOR_CPU:
            if (command == "CPU")
            {
                return ProtocolState::WAITING_FOR_RAM;
            }

            return std::nullopt;

        case ProtocolState::WAITING_FOR_RAM:
            if (command == "RAM")
            {
                return ProtocolState::WAITING_FOR_DISK_FREE;
            }

            return std::nullopt;

        case ProtocolState::WAITING_FOR_DISK_FREE:
            if (command == "DISK_FREE")
            {
                return ProtocolState::WAITING_FOR_ROBOT_RUNNING;
            }

            return std::nullopt;

        case ProtocolState::WAITING_FOR_ROBOT_RUNNING:
            if (command == "ROBOT_RUNNING")
            {
                return ProtocolState::WAITING_FOR_UPTIME;
            }

            return std::nullopt;

        case ProtocolState::WAITING_FOR_UPTIME:
            if (command == "UPTIME")
            {
                return ProtocolState::WAITING_FOR_DONE;
            }

            return std::nullopt;

        case ProtocolState::WAITING_FOR_DONE:
            if (command == "DONE")
            {
                return ProtocolState::COMPLETE;
            }

            return std::nullopt;

        case ProtocolState::COMPLETE:
            return std::nullopt;
    }

    return std::nullopt;
}



/**
 * Get the protocol command expected for a state.
 *
 * Args:
 *     state: Protocol state to translate.
 *
 * Returns:
 *     The command token corresponding to the supplied protocol state.
 */
std::string ProtocolStateMachine::expected_command(
    ProtocolState state
)
{
    switch (state)
    {
        case ProtocolState::WAITING_FOR_INITIAL_MESSAGE:
            return "HELLO";

        case ProtocolState::WAITING_FOR_CPU:
            return "CPU";

        case ProtocolState::WAITING_FOR_RAM:
            return "RAM";

        case ProtocolState::WAITING_FOR_DISK_FREE:
            return "DISK_FREE";

        case ProtocolState::WAITING_FOR_ROBOT_RUNNING:
            return "ROBOT_RUNNING";

        case ProtocolState::WAITING_FOR_UPTIME:
            return "UPTIME";

        case ProtocolState::WAITING_FOR_DONE:
            return "DONE";

        case ProtocolState::COMPLETE:
            return "COMPLETE";
    }

    return "";
}
