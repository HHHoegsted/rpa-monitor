#pragma once

#include <winsock2.h>


class Socket
{
public:
    explicit Socket(SOCKET socket_handle);

    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    SOCKET get() const;

    bool is_valid() const;

private:
    SOCKET socket_handle_;
};