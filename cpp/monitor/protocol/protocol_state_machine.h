#pragma once

#include <optional>
#include <string>

#include "protocol_state.h"

class ProtocolStateMachine
{
public:
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
    static std::optional<ProtocolState> next_state(
        ProtocolState current_state,
        const std::string& command
    );

    /**
     * Get the protocol command expected for a state.
     *
     * Args:
     *     state: Protocol state to translate.
     *
     * Returns:
     *     The command token expected by the client for the supplied state.
     */
    static std::string expected_command(
        ProtocolState state
    );
};