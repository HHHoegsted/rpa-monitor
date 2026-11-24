#include "network_session.h"

#ifdef _WIN32
#include <winsock2.h>
#include <iostream>
#endif


/**
 * Initialise the platform-specific networking subsystem.
 *
 * Windows requires explicit Winsock initialisation.
 * Linux requires no explicit initialisation.
 */
NetworkSession::NetworkSession()
    : initialized_(false)
{
#ifdef _WIN32
    WSADATA winsock_data{};

    const int startup_result = WSAStartup(
        MAKEWORD(2, 2),
        &winsock_data
    );

    if (startup_result != 0)
    {
        std::cerr << "WSAStartup failed with error: "
                  << startup_result
                  << '\n';

        return;
    }
#endif

    initialized_ = true;
}


/**
 * Release resources associated with the networking subsystem
 * if initialisation succeeded.
 */
NetworkSession::~NetworkSession()
{
#ifdef _WIN32
    if (initialized_)
    {
        WSACleanup();
    }
#endif
}


/**
 * Determine whether the networking subsystem
 * was successfully initialised.
 *
 * Returns:
 *     True if networking is available, otherwise false.
 */
bool NetworkSession::is_valid() const noexcept
{
    return initialized_;
}