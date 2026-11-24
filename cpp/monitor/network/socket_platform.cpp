#include "socket_platform.h"

#ifndef _WIN32
#include <unistd.h>
#include <cerrno>
#endif


/**
 * Close a socket using the appropriate platform-specific operation.
 *
 * Args:
 *     socket_handle: Handle of the socket to close.
 */
void close_socket(SocketHandle socket_handle) noexcept
{
#ifdef _WIN32
    closesocket(socket_handle);
#else
    close(socket_handle);
#endif
}


/**
 * Retrieve the most recent socket error code
 * for the current thread.
 *
 * Returns:
 *     Platform-specific error code.
 */
int get_socket_error() noexcept
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}