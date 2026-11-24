cmake_minimum_required(VERSION 3.20)

project(rpa-monitor LANGUAGES CXX)

find_package(libpqxx CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Shared networking library

add_library(
    monitor_network STATIC
    monitor/network/socket.cpp
    monitor/network/socket_platform.cpp
    monitor/network/network_session.cpp
)

if(WIN32)
    target_sources(
        monitor_network
        PRIVATE
        monitor/network/winsock_session.cpp
    )

    target_link_libraries(
        monitor_network
        PUBLIC
        Ws2_32
    )
endif()


# Cross-platform monitoring server

add_executable(
    server
    monitor/server.cpp
    monitor/protocol/message.cpp
    monitor/protocol/dispatcher.cpp
    monitor/protocol/protocol_state_machine.cpp
    monitor/models/machine_report.cpp
    monitor/database/report_repository.cpp
)

target_link_libraries(
    server
    PRIVATE
    monitor_network
    libpqxx::pqxx
)


# Windows monitoring client

if(WIN32)
    add_executable(
        client
        monitor/client.cpp
        monitor/metrics/system_metrics.cpp
        monitor/metrics/robot_status.cpp
    )

    target_link_libraries(
        client
        PRIVATE
        monitor_network
        WbemUuid
        Ole32
        OleAut32
    )
endif()