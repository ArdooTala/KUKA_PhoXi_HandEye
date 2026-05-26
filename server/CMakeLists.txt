cmake_minimum_required(VERSION 3.10)

project(TcpServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions(-DASIO_STANDALONE)

find_package(Threads REQUIRED)

add_library(tcp_server_lib STATIC tcp_server.cpp tcp_server.h)
target_include_directories(tcp_server_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/asio/asio/include)

if(WIN32)
    target_link_libraries(tcp_server_lib PUBLIC ws2_32)
else()
    target_link_libraries(tcp_server_lib PUBLIC Threads::Threads)
endif()

add_executable(test_server main.cpp)
target_link_libraries(test_server PRIVATE tcp_server_lib)

