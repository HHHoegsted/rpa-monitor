#pragma once

#include "socket_platform.h"


class Socket
{
public:
    explicit Socket(SocketHandle socket_handle);

    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    SocketHandle get() const;

    bool is_valid() const;

private:
    SocketHandle socket_handle_;
};