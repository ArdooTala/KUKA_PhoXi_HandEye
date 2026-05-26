#define PHOXI_PCL_SUPPORT

#include "PhoXi.h"
#include "detector/marker_detector.h"
#include "kuka_utils/kuka_utils.h"
#include "pugixml.hpp"
#include "server/tcp_server.h"
#include <iostream>
#include <pcl/common/transforms.h>
#include <pcl/io/ply_io.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <string>

#define PORT 59153

pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr
convertToPCL(const pho::api::PFrame &Frame) {
  std::cout << "Frame " << Frame << "\n";

  pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr PCLCloud(
      new pcl::PointCloud<pcl::PointXYZRGBNormal>());
  Frame->ConvertTo(*PCLCloud);

  std::cout << PCLCloud->points.size() << " points in the PCL Cloud : " << std::endl;
  return PCLCloud;
}

int main(int argc, char *argv[]) {
  std::string hwId;

  if (argc > 1) {
    std::cout << "Connection HWID: " << argv[1] << std::endl;
    hwId = argv[1];

    if (hwId.empty()) {
      std::cout << "Error reading the HardwareID" << std::endl;
      return 1;
    }
  } else {
    hwId = "PAD-104";
  }
  std::cout.precision(std::numeric_limits<double>::max_digits10 - 1);

  markerDetection::PhoXiCam camera(hwId);
  camera.InitDevice(false);

  TcpServer server(PORT);

  if (!server.start()) {
    std::cerr << "Failed to start the server" << std::endl;
    return -1;
  }

  if (server.acceptClient() < 0) {
    std::cerr << "Failed to accept client." << std::endl;
    server.stop();
    return -1;
  }

  // A client has connected!
  std::cout << "Client connected!" << std::endl;
  std::cout << "Client IP: " << server.getClientIp() << std::endl;

  std::string msg;
  int index = 0;
  while (!(msg = server.receiveMessage()).empty()) {
    std::cout << msg << std::endl;

    if (msg.find("<Position") == std::string::npos)
      break;
    auto rob_pos = KukaUtils::E6POS(msg);
    auto rob_transform = (Eigen::Isometry3f)rob_pos;

    std::cout << ">>> Robot TCP Position:" << std::endl
              << rob_transform.matrix() << std::endl;

    pho::api::PFrame frame;
    try {
      frame = camera.Trigger();
    } catch (...) {
      std::cerr << "Capture failed!..." << std::endl;
    }

    server.sendMessage("<BasicRecv><Flag12></Flag12></BasicRecv>");

    auto source_cloud = convertToPCL(frame);
    pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr transformed_cloud(
        new pcl::PointCloud<pcl::PointXYZRGBNormal>());
    pcl::transformPointCloud(*source_cloud, *transformed_cloud, rob_transform);

    std::string file_name = "/home/ardeshir/Desktop/TestCaptures/capture_" + std::to_string(index) + ".ply";
    pcl::io::savePLYFile(file_name, *transformed_cloud);
    index++;
  }

  while (!server.receiveMessage().empty())
    ;

  return 0;
}
