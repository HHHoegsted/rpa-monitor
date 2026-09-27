#pragma once

#include <string>


/**
 * Represents one parsed protocol message.
 */
class ProtocolMessage
{
public:
    /**
     * Create a protocol message from raw message text.
     *
     * Args:
     *     raw_message: Complete unframed protocol message to parse.
     */
    explicit ProtocolMessage(const std::string& raw_message);

    /**
     * Check whether the message could be parsed.
     *
     * Returns:
     *     True if the message contains a valid command, otherwise false.
     */
    bool is_valid() const;

    /**
     * Get the protocol command.
     *
     * Returns:
     *     The parsed command.
     */
    const std::string& command() const;

    /**
     * Get the protocol payload.
     *
     * Returns:
     *     Everything following the command.
     */
    const std::string& payload() const;

private:
    /**
     * Parse raw protocol message text.
     *
     * Args:
     *     raw_message: Complete unframed protocol message to parse.
     */
    void parse(const std::string& raw_message);

    std::string command_;
    std::string payload_;
    bool valid_ = false;
};