#pragma once

#include <mosquitto.h>


namespace mqtt
{
    /**
     * Manage the lifetime of the libmosquitto library
     * through RAII.
     */
    class LibrarySession
    {
    public:
        /**
         * Initialise the libmosquitto library.
         */
        LibrarySession()
            : result_(mosquitto_lib_init())
        {
        }

        /**
         * Release library resources if initialisation succeeded.
         */
        ~LibrarySession()
        {
            if (is_valid())
            {
                mosquitto_lib_cleanup();
            }
        }

        LibrarySession(const LibrarySession&) = delete;
        LibrarySession& operator=(const LibrarySession&) = delete;

        LibrarySession(LibrarySession&&) = delete;
        LibrarySession& operator=(LibrarySession&&) = delete;

        /**
         * Determine whether library initialisation succeeded.
         *
         * Returns:
         *     True if the library is available, otherwise false.
         */
        bool is_valid() const noexcept
        {
            return result_ == MOSQ_ERR_SUCCESS;
        }

    private:
        int result_;
    };
}