#include "winsock_session.h"

#include <iostream>


WinsockSession::WinsockSession()
    : initialized_(false)
{
    WSADATA winsock_data;

    int startup_result = WSAStartup(MAKEWORD(2, 2), &winsock_data);

    if (startup_result != 0)
    {
        std::cerr << "WSAStartup failed with error: "
                  << startup_result
                  << '\n';

        return;
    }

    initialized_ = true;
}


WinsockSession::~WinsockSession()
{
    if (initialized_)
    {
        WSACleanup();
    }
}


bool WinsockSession::is_valid() const
{
    return initialized_;
}