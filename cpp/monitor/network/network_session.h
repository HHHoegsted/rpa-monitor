#pragma once


class NetworkSession
{
public:
    NetworkSession();

    ~NetworkSession();

    NetworkSession(const NetworkSession&) = delete;
    NetworkSession& operator=(const NetworkSession&) = delete;

    NetworkSession(NetworkSession&&) = delete;
    NetworkSession& operator=(NetworkSession&&) = delete;

    bool is_valid() const noexcept;

private:
    bool initialized_;
};