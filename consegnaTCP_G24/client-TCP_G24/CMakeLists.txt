cmake_minimum_required(VERSION 4.3)
project(client_TCP_G24 C)

set(CMAKE_C_STANDARD 11)

add_executable(client_TCP_G24 client-TCP_G24.c)

if(WIN32)
    target_link_libraries(client_TCP_G24 PRIVATE ws2_32)
endif()