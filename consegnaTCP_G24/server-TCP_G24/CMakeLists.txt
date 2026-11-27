cmake_minimum_required(VERSION 4.3)
project(server_TCP_G24 C)

set(CMAKE_C_STANDARD 11)

add_executable(server_TCP_G24 server-TCP_G24.c)

if(WIN32)
    target_link_libraries(server_TCP_G24 PRIVATE ws2_32)
endif()