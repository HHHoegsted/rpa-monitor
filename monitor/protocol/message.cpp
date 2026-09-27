#include "message.h"


/**
 * Create a protocol message from raw message text.
 *
 * Args:
 *     raw_message: Complete unframed protocol message to parse.
 */
ProtocolMessage::ProtocolMessage(const std::string& raw_message)
{
    parse(raw_message);
}


/**
 * Check whether the message could be parsed.
 *
 * Returns:
 *     True if the message contains a valid command, otherwise false.
 */
bool ProtocolMessage::is_valid() const
{
    return valid_;
}


/**
 * Get the protocol command.
 *
 * Returns:
 *     The parsed command.
 */
const std::string& ProtocolMessage::command() const
{
    return command_;
}


/**
 * Get the protocol payload.
 *
 * Returns:
 *     Everything following the command.
 */
const std::string& ProtocolMessage::payload() const
{
    return payload_;
}


/**
 * Parse raw protocol message text.
 *
 * Args:
 *     raw_message: Complete unframed protocol message to parse.
 */
void ProtocolMessage::parse(const std::string& raw_message)
{
    if (raw_message.empty())
    {
        return;
    }

    std::size_t separator_position = raw_message.find(' ');

    if (separator_position == std::string::npos)
    {
        command_ = raw_message;
        valid_ = true;
        return;
    }

    command_ = raw_message.substr(0, separator_position);
    payload_ = raw_message.substr(separator_position + 1);

    valid_ = !command_.empty();
}