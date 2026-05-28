cmake_minimum_required(VERSION 3.10)

project(KukaPhoXiHandEyeCalibration)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(server)
add_subdirectory(detector)
add_subdirectory(kuka_utils)
add_subdirectory(handeye)

find_package(PCL REQUIRED)

add_executable(HandEyeCalibration)

target_sources(HandEyeCalibration
    PRIVATE
        HandEyeCalibration.cpp
)

target_link_libraries(HandEyeCalibration
    PUBLIC
        tcp_server_lib
        marker_detector_lib
        KukaUtils
        handeye_lib
)

add_executable(PointCloudCapture)

target_sources(PointCloudCapture
    PRIVATE
        PointCloudCapture.cpp
)

target_link_libraries(PointCloudCapture
    PUBLIC
        tcp_server_lib
        marker_detector_lib
        KukaUtils
        ${PCL_LIBRARIES}
)
