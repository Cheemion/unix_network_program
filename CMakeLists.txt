cmake_minimum_required(VERSION 3.16)
project(NETWORK_PROGRAMMING)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

include_directories(./include)
link_directories(./lib)
add_executable(main main.cpp)
add_executable(tcpcli tcpcli_select.c)
add_executable(tcpserv tcpserv_select.c)