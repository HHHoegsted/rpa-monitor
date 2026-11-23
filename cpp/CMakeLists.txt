cmake_minimum_required(VERSION 3.20)

project(rpa-monitor LANGUAGES CXX)

find_package(libpqxx CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_executable(
    server
    monitor/server.cpp
    monitor/network/socket.cpp
    monitor/network/winsock_session.cpp
    monitor/protocol/message.cpp
    monitor/protocol/dispatcher.cpp
    monitor/protocol/protocol_state_machine.cpp
    monitor/models/machine_report.cpp
    monitor/database/report_repository.cpp
)

target_link_libraries(
    server
    PRIVATE
    Ws2_32
    libpqxx::pqxx
)


add_executable(
    client
    monitor/client.cpp
    monitor/network/socket.cpp
    monitor/network/winsock_session.cpp
    monitor/metrics/system_metrics.cpp
    monitor/metrics/robot_status.cpp
)

target_link_libraries(
    client
    PRIVATE
    Ws2_32
    WbemUuid
    Ole32
    OleAut32
)