cmake_minimum_required(VERSION 3.10)

project(hand_eye)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

add_library(handeye_lib STATIC
    handeye.cpp
    handeye.h
)

target_include_directories(handeye_lib
    PUBLIC
        ${OpenCV_INCLUDE_DIR}
        ${EIGEN3_INCLUDE_DIR}
)

target_link_libraries(handeye_lib
    PUBLIC
        ${OpenCV_LIBS}
)

