#pragma once

#include <winsock2.h>


class WinsockSession
{
public:
    WinsockSession();

    ~WinsockSession();

    WinsockSession(const WinsockSession&) = delete;
    WinsockSession& operator=(const WinsockSession&) = delete;

    WinsockSession(WinsockSession&&) = delete;
    WinsockSession& operator=(WinsockSession&&) = delete;

    bool is_valid() const;

private:
    bool initialized_;
};