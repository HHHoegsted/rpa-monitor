#include <iostream>
#include <string>
#include <winsock2.h>

#include "network/socket.h"
#include "network/winsock_session.h"
#include "metrics/system_metrics.h"
#include "metrics/robot_status.h"


#pragma comment(lib, "Ws2_32.lib")


/**
 * Initialize the Windows Sockets API.
 *
 * Returns:
 *     True if Winsock was initialized successfully, otherwise false.
 */
bool initialize_winsock()
{
    WSADATA winsock_data;

    int startup_result = WSAStartup(MAKEWORD(2, 2), &winsock_data);

    if (startup_result != 0)
    {
        std::cerr << "WSAStartup failed with error: "
                  << startup_result
                  << '\n';

        return false;
    }

    return true;
}


/**
 * Get the hostname of the local machine.
 *
 * Returns:
 *     The local hostname, or an empty string if retrieval fails.
 */
std::string get_hostname()
{
    char hostname[256]{};

    int result = gethostname(
        hostname,
        static_cast<int>(sizeof(hostname))
    );

    if (result == SOCKET_ERROR)
    {
        std::cerr << "gethostname() failed with error: "
                  << WSAGetLastError()
                  << '\n';

        return "";
    }

    return hostname;
}


/**
 * Create an IPv4 TCP socket.
 *
 * Returns:
 *     A Socket object owning the created socket handle.
 */
Socket create_client_socket()
{
    SOCKET socket_handle = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP
    );

    if (socket_handle == INVALID_SOCKET)
    {
        std::cerr << "socket() failed with error: "
                  << WSAGetLastError()
                  << '\n';
    }

    return Socket(socket_handle);
}


/**
 * Connect a socket to the local test server.
 *
 * Args:
 *     client_socket: Socket used for the connection.
 *
 * Returns:
 *     True if the connection succeeds, otherwise false.
 */
bool connect_to_server(Socket& client_socket)
{
    sockaddr_in server_address{};

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_address.sin_port = htons(5000);

    int connect_result = connect(
        client_socket.get(),
        reinterpret_cast<sockaddr*>(&server_address),
        sizeof(server_address)
    );

    if (connect_result == SOCKET_ERROR)
    {
        std::cerr << "connect() failed with error: "
                  << WSAGetLastError()
                  << '\n';

        return false;
    }

    return true;
}


/**
 * Send a complete message through a connected socket.
 *
 * Args:
 *     client_socket: Connected socket used to send the message.
 *     message: Message to send.
 *
 * Returns:
 *     True if the send operation succeeds, otherwise false.
 */
/**
 * Send a complete message through a connected socket.
 *
 * Args:
 *     client_socket: Connected socket used to send the message.
 *     message: Message to send.
 *
 * Returns:
 *     True if the send operation succeeds, otherwise false.
 */
bool send_message(
    Socket& client_socket,
    const std::string& message
)
{
    std::size_t total_sent = 0;

    while (total_sent < message.size())
    {
        int sent_bytes = send(
            client_socket.get(),
            message.c_str() + total_sent,
            static_cast<int>(message.size() - total_sent),
            0
        );

        if (sent_bytes == SOCKET_ERROR)
        {
            std::cerr << "send() failed with error: "
                      << WSAGetLastError()
                      << '\n';

            return false;
        }

        if (sent_bytes == 0)
        {
            std::cerr << "send() made no progress.\n";
            return false;
        }

        total_sent += static_cast<std::size_t>(sent_bytes);
    }

    std::cout << "Sent " << total_sent << " bytes.\n";

    return true;
}


/**
 * Receive one newline-terminated response from the server.
 *
 * Args:
 *     client_socket: Connected socket used to receive the response.
 *     response: String populated with the complete response.
 *
 * Returns:
 *     True if a complete response was received, otherwise false.
 */
bool receive_response(
    Socket& client_socket,
    std::string& response
)
{
    std::string receive_buffer;

    while (true)
    {
        std::size_t newline_position = receive_buffer.find('\n');

        if (newline_position != std::string::npos)
        {
            response = receive_buffer.substr(0, newline_position);
            return true;
        }

        char buffer[1024]{};

        int received_bytes = recv(
            client_socket.get(),
            buffer,
            static_cast<int>(sizeof(buffer)),
            0
        );

        if (received_bytes == SOCKET_ERROR)
        {
            std::cerr << "recv() failed with error: "
                      << WSAGetLastError()
                      << '\n';

            return false;
        }

        if (received_bytes == 0)
        {
            std::cerr << "Server closed the connection before sending a complete response.\n";
            return false;
        }

        receive_buffer.append(
            buffer,
            static_cast<std::size_t>(received_bytes)
        );
    }
}


/**
 * Send a protocol message and receive the server response.
 *
 * Args:
 *     client_socket: Connected socket used for the exchange.
 *     message: Protocol message to send.
 *
 * Returns:
 *     The server response when the exchange succeeds, otherwise no value.
 */
std::optional<std::string> exchange_message(
    Socket& client_socket,
    const std::string& message
)
{
    if (!send_message(client_socket, message))
    {
        return std::nullopt;
    }

    std::string response;

    if (!receive_response(client_socket, response))
    {
        return std::nullopt;
    }

    std::cout << "Received response: " << response << '\n';

    return response;
}


/**
 * Create the protocol message requested by the server.
 *
 * Args:
 *     command: Protocol command requested by the server.
 *
 * Returns:
 *     The complete protocol message when the requested value can be
 *     obtained, otherwise no value.
 */
std::optional<std::string> create_requested_message(
    const std::string& command
)
{
    if (command == "CPU")
    {
        std::optional<double> cpu_percent =
            system_metrics::get_cpu_percent();

        if (!cpu_percent.has_value())
        {
            std::cerr << "Could not measure CPU usage.\n";
            return std::nullopt;
        }

        std::cout << "CPU usage: "
                  << cpu_percent.value()
                  << "%\n";

        return "CPU "
            + std::to_string(cpu_percent.value())
            + "\n";
    }

    if (command == "RAM")
    {
        std::optional<double> ram_percent =
            system_metrics::get_ram_percent();

        if (!ram_percent.has_value())
        {
            std::cerr << "Could not measure RAM usage.\n";
            return std::nullopt;
        }

        std::cout << "RAM usage: "
                  << ram_percent.value()
                  << "%\n";

        return "RAM "
            + std::to_string(ram_percent.value())
            + "\n";
    }

    if (command == "DISK_FREE")
    {
        std::optional<double> disk_free_percent =
            system_metrics::get_disk_free_percent();

        if (!disk_free_percent.has_value())
        {
            std::cerr << "Could not measure free disk space.\n";
            return std::nullopt;
        }

        std::cout << "Disk free: "
                  << disk_free_percent.value()
                  << "%\n";

        return "DISK_FREE "
            + std::to_string(disk_free_percent.value())
            + "\n";
    }

    if (command == "ROBOT_RUNNING")
    {
        std::optional<bool> robot_running =
            robot_status::is_robot_running();

        if (!robot_running.has_value())
        {
            std::cerr
                << "Failed to determine whether a robot is running.\n";

            return std::nullopt;
        }

        return "ROBOT_RUNNING "
            + std::string(
                robot_running.value()
                    ? "true"
                    : "false"
            )
            + "\n";
    }

    if (command == "UPTIME")
    {
        unsigned long long uptime_seconds =
            system_metrics::get_uptime_seconds();

        std::cout << "Uptime: "
                  << uptime_seconds
                  << " seconds\n";

        return "UPTIME "
            + std::to_string(uptime_seconds)
            + "\n";
    }

    if (command == "DONE")
    {
        return "DONE\n";
    }

    std::cerr
        << "Server requested unsupported command: "
        << command
        << '\n';

    return std::nullopt;
}



/**
 * Run the TCP client conversation.
 *
 * Returns:
 *     0 when the protocol conversation completes successfully, otherwise 1.
 */
int run_client()
{
    Socket client_socket = create_client_socket();

    if (!client_socket.is_valid())
    {
        return 1;
    }

    if (!connect_to_server(client_socket))
    {
        return 1;
    }

    std::cout << "Connected to server.\n";

    std::string hostname = get_hostname();

    if (hostname.empty())
    {
        return 1;
    }

    std::optional<std::string> response = exchange_message(
        client_socket,
        "HELLO " + hostname + "\n"
    );

    if (!response.has_value())
    {
        return 1;
    }

    while (true)
    {
        if (response.value() == "OK COMPLETE")
        {
            return 0;
        }

        if (response.value().rfind("OK ", 0) != 0)
        {
            std::cerr
                << "Server rejected protocol message: "
                << response.value()
                << '\n';

            return 1;
        }

        std::string requested_command =
            response.value().substr(3);

        std::optional<std::string> message =
            create_requested_message(requested_command);

        if (!message.has_value())
        {
            return 1;
        }

        response = exchange_message(
            client_socket,
            message.value()
        );

        if (!response.has_value())
        {
            return 1;
        }
    }
}

/**
 * Initialize networking and run the client.
 *
 * Returns:
 *     0 on success, otherwise 1.
 */
int main()
{
    WinsockSession winsock;

    if (!winsock.is_valid())
    {
        return 1;
    }

    return run_client();
}