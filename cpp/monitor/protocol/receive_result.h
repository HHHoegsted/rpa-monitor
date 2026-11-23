#pragma once


/**
 * Describes the result of receiving a framed protocol message.
 */
enum class ReceiveResult
{
    MessageReceived,
    ConnectionClosed,
    Error
};