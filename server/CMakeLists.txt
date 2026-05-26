cmake_minimum_required(VERSION 3.10)

project(TcpServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(tcp_server_lib STATIC tcp_server.cpp tcp_server.h)
if(WIN32)
    target_link_libraries(tcp_server_lib PUBLIC ws2_32)
endif()

add_executable(test_server main.cpp)
target_link_libraries(test_server PRIVATE tcp_server_lib)

