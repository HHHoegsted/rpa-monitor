#pragma once

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

using SocketHandle = SOCKET;

constexpr SocketHandle INVALID_SOCKET_HANDLE = INVALID_SOCKET;
constexpr int SOCKET_OPERATION_ERROR = SOCKET_ERROR;
constexpr int SOCKET_SEND_FLAGS = 0;

#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using SocketHandle = int;

constexpr SocketHandle INVALID_SOCKET_HANDLE = -1;
constexpr int SOCKET_OPERATION_ERROR = -1;
constexpr int SOCKET_SEND_FLAGS = MSG_NOSIGNAL;

#endif


void close_socket(SocketHandle socket_handle) noexcept;

int get_socket_error() noexcept;