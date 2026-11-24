cmake_minimum_required(VERSION 3.20)

project(rpa-monitor LANGUAGES CXX)

find_package(libpqxx CONFIG REQUIRED)
find_package(unofficial-mosquitto CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Shared MQTT client infrastructure

add_library(
    monitor_mqtt STATIC
    monitor/mqtt/client_session.cpp
)

target_link_libraries(
    monitor_mqtt
    PUBLIC
    unofficial::mosquitto::mosquitto
)


# Cross-platform MQTT collector

add_executable(
    server
    monitor/server.cpp
    monitor/mqtt/subscriber.cpp
    monitor/mqtt/report_parser.cpp
    monitor/models/machine_report.cpp
    monitor/database/report_repository.cpp
)

target_link_libraries(
    server
    PRIVATE
    monitor_mqtt
    libpqxx::pqxx
    nlohmann_json::nlohmann_json
)


# Windows MQTT monitoring client

if(WIN32)
    add_executable(
        client
        monitor/client.cpp
        monitor/metrics/system_metrics.cpp
        monitor/metrics/robot_status.cpp
        monitor/mqtt/publisher.cpp
    )

    target_link_libraries(
        client
        PRIVATE
        monitor_mqtt
        WbemUuid
        Ole32
        OleAut32
    )
endif()