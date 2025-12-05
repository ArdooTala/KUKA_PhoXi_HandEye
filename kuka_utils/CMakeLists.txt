cmake_minimum_required(VERSION 3.12)
project(KukaUtils CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Eigen3 REQUIRED)

add_library(KukaUtils STATIC kuka_utils.cpp kuka_utils.h)

target_include_directories(KukaUtils PUBLIC ${EIGEN3_INCLUDE_DIR})
