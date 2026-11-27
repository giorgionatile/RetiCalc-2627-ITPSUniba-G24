cmake_minimum_required(VERSION 4.3)
project(serverTCP_G24 C)

set(CMAKE_C_STANDARD 11)

add_executable(serverTCP_G24 serverTCP_G24.c)

if(WIN32)
    target_link_libraries(serverTCP_G24 PRIVATE ws2_32)
endif()