cmake_minimum_required(VERSION 3.10)

project(marker_detector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(marker_detector_lib STATIC marker_detector.cpp marker_detector.h)

if(NOT PHO_API_CMAKE_CONFIG_PATH)
    set(PHO_API_CMAKE_CONFIG_PATH "$ENV{PHOXI_CONTROL_PATH}")
endif()

if(NOT PHO_BUILT_IN_API_IN_EXAMPLES)
    find_package(PhoXi REQUIRED CONFIG PATHS "${PHO_API_CMAKE_CONFIG_PATH}")
endif()

target_link_libraries(marker_detector_lib
    PUBLIC
        ${PHOXI_LIBRARY}
)

target_include_directories(marker_detector_lib
    PUBLIC
        ${PHOXI_INCLUDE_DIRS}
)

