#include "socket.h"

#include <utility>


Socket::Socket(SocketHandle socket_handle)
    : socket_handle_(socket_handle)
{
}


Socket::~Socket()
{
    if (is_valid())
    {
        close_socket(socket_handle_);
    }
}


Socket::Socket(Socket&& other) noexcept
    : socket_handle_(other.socket_handle_)
{
    other.socket_handle_ = INVALID_SOCKET_HANDLE;
}


Socket& Socket::operator=(Socket&& other) noexcept
{
    if (this != &other)
    {
        if (is_valid())
        {
            close_socket(socket_handle_);
        }

        socket_handle_ = other.socket_handle_;
        other.socket_handle_ = INVALID_SOCKET_HANDLE;
    }

    return *this;
}


SocketHandle Socket::get() const
{
    return socket_handle_;
}


bool Socket::is_valid() const
{
    return socket_handle_ != INVALID_SOCKET_HANDLE;
}