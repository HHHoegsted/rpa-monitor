#include <iostream>
#include <string>
#include <winsock2.h>

#include "network/socket.h"
#include "network/winsock_session.h"
#include "protocol/receive_result.h"
#include "protocol/message.h"
#include "protocol/dispatcher.h"
#include "protocol/protocol_state.h"
#include "models/machine_report.h"
#include "database/report_repository.h"
#include <pqxx/pqxx>
#include <exception>


#pragma comment(lib, "Ws2_32.lib")


/**
 * Create an IPv4 TCP socket for the server.
 *
 * Returns:
 *     A Socket object owning the created socket handle.
 */
Socket create_server_socket()
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
 * Bind the server socket to localhost on port 5000.
 *
 * Args:
 *     server_socket: Socket to bind.
 *
 * Returns:
 *     True if binding succeeds, otherwise false.
 */
bool bind_server_socket(Socket& server_socket)
{
    sockaddr_in server_address{};

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_address.sin_port = htons(5000);

    int bind_result = bind(
        server_socket.get(),
        reinterpret_cast<sockaddr*>(&server_address),
        sizeof(server_address)
    );

    if (bind_result == SOCKET_ERROR)
    {
        std::cerr << "bind() failed with error: "
                  << WSAGetLastError()
                  << '\n';

        return false;
    }

    return true;
}


/**
 * Put the server socket into listening mode.
 *
 * Args:
 *     server_socket: Bound socket that should listen for connections.
 *
 * Returns:
 *     True if listening starts successfully, otherwise false.
 */
bool listen_for_connections(Socket& server_socket)
{
    int listen_result = listen(
        server_socket.get(),
        SOMAXCONN
    );

    if (listen_result == SOCKET_ERROR)
    {
        std::cerr << "listen() failed with error: "
                  << WSAGetLastError()
                  << '\n';

        return false;
    }

    return true;
}


/**
 * Accept one incoming client connection.
 *
 * Args:
 *     server_socket: Listening server socket.
 *
 * Returns:
 *     A Socket object owning the accepted client connection.
 */
Socket accept_client(Socket& server_socket)
{
    SOCKET client_handle = accept(
        server_socket.get(),
        nullptr,
        nullptr
    );

    if (client_handle == INVALID_SOCKET)
    {
        std::cerr << "accept() failed with error: "
                  << WSAGetLastError()
                  << '\n';
    }

    return Socket(client_handle);
}


/**
 * Receive one newline-terminated message from a connected client.
 *
 * Any bytes received after the first newline are kept in the receive buffer
 * for the next message. A carriage return immediately preceding the newline
 * is removed so both LF and CRLF line endings are accepted.
 *
 * Args:
 *     client_socket: Connected client socket.
 *     receive_buffer: Buffered data received from the client but not yet consumed.
 *     message: String populated with one complete protocol message.
 *
 * Returns:
 *     The result of the receive operation.
 */
ReceiveResult receive_message(
    Socket& client_socket,
    std::string& receive_buffer,
    std::string& message
)
{
    while (true)
    {
        std::size_t newline_position = receive_buffer.find('\n');

        if (newline_position != std::string::npos)
        {
            message = receive_buffer.substr(0, newline_position);

            if (!message.empty() && message.back() == '\r')
            {
                message.pop_back();
            }

            receive_buffer.erase(
                0,
                newline_position + 1
            );

            return ReceiveResult::MessageReceived;
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

            return ReceiveResult::Error;
        }

        if (received_bytes == 0)
        {
            return ReceiveResult::ConnectionClosed;
        }

        receive_buffer.append(
            buffer,
            static_cast<std::size_t>(received_bytes)
        );
    }
}


/**
 * Send one newline-terminated response through a connected client socket.
 *
 * Args:
 *     client_socket: Connected client socket.
 *     response: Response to send.
 *
 * Returns:
 *     True if sending succeeds, otherwise false.
 */

/**
 * Send one complete newline-terminated response through a client socket.
 *
 * Args:
 *     client_socket: Connected socket used to send the response.
 *     response: Protocol response to send.
 *
 * Returns:
 *     True if every byte was sent successfully, otherwise false.
 */
bool send_response(
    Socket& client_socket,
    const std::string& response
)
{
    const std::string framed_response = response + "\n";
    std::size_t total_sent = 0;

    while (total_sent < framed_response.size())
    {
        int sent_bytes = send(
            client_socket.get(),
            framed_response.c_str() + total_sent,
            static_cast<int>(framed_response.size() - total_sent),
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

    std::cout << "Sent response: " << response << '\n';

    return true;
}




/**
 * Handle one connected client conversation.
 *
 * Persists a complete report before acknowledging successful
 * completion of the monitoring protocol.
 *
 * Args:
 *     client_socket: Socket connected to the client.
 *     dispatcher: Dispatcher responsible for protocol validation.
 *
 * Returns:
 *     True if the conversation completed successfully,
 *     otherwise false.
 */
bool handle_client(
    Socket& client_socket,
    const ProtocolDispatcher& dispatcher
)
{
    std::string receive_buffer;
    MachineReport report;
    ProtocolState state = ProtocolState::WAITING_FOR_INITIAL_MESSAGE;

    while (true)
    {
        std::string message;

        ReceiveResult receive_result = receive_message(
            client_socket,
            receive_buffer,
            message
        );

        if (receive_result == ReceiveResult::ConnectionClosed)
        {
            if (state != ProtocolState::COMPLETE)
            {
                std::cerr << "Client closed the connection before completing the protocol.\n";
                return false;
            }

            std::cout << "Client closed the connection.\n";
            return true;
        }

        if (receive_result == ReceiveResult::Error)
        {
            return false;
        }

        std::cout << "Received: " << message << '\n';

        ProtocolMessage protocol_message(message);

        std::string response = dispatcher.dispatch(
            protocol_message,
            report,
            state
        );

        if (
            protocol_message.command() == "DONE"
            && response == "OK COMPLETE"
        )
        {
            if (!report.is_complete() || !save_machine_report(report))
            {
                send_response(
                    client_socket,
                    "ERROR database persistence failed"
                );

                return false;
            }

            std::cout << "Saved machine report: "
                      << report.hostname()
                      << '\n';
        }

        if (!send_response(client_socket, response))
        {
            return false;
        }
    }
}




/**
 * Verify that the PostgreSQL database is accessible.
 *
 * Returns:
 *     True if the connection and test query succeed,
 *     otherwise false.
 */
bool verify_database_connection()
{
    try
    {
        pqxx::connection connection;

        pqxx::work transaction{connection};

        pqxx::result result = transaction.exec("SELECT 1");

        transaction.commit();

        return result.size() == 1
            && result[0][0].as<int>() == 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Database connection failed: "
                  << error.what()
                  << '\n';

        return false;
    }
}



/**
 * Starts the server and handles clients sequentially.
 *
 * Returns:
 *     0 if the server exits successfully, otherwise 1.
 */
int run_server()
{
    Socket server_socket = create_server_socket();

    if (!server_socket.is_valid())
    {
        return 1;
    }

    if (!bind_server_socket(server_socket))
    {
        return 1;
    }

    if (!listen_for_connections(server_socket))
    {
        return 1;
    }

    ProtocolDispatcher dispatcher;

    std::cout << "Server listening on 127.0.0.1:5000...\n";

    while (true)
    {
        std::cout << "Waiting for client...\n";

        Socket client_socket = accept_client(server_socket);

        if (!client_socket.is_valid())
        {
            std::cerr << "Server could not accept client. Exiting.\n";
            return 1;
        }

        std::cout << "Client connected.\n";

        if (!handle_client(client_socket, dispatcher))
        {
            std::cerr << "Client conversation failed. Returning to accept().\n";
            continue;
        }

        std::cout << "Client conversation completed successfully.\n";
    }

    return 0;
}



/**
 * Initialize networking, verify database connectivity,
 * and start the monitoring server.
 *
 * Returns:
 *     0 on successful termination, otherwise 1.
 */
int main()
{
    WinsockSession winsock;

    if (!winsock.is_valid())
    {
        return 1;
    }

    if (!verify_database_connection())
    {
        return 1;
    }

    return run_server();
}
