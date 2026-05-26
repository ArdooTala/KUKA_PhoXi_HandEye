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
#include <vector>

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " [options] [hardware_id]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -i, --hw-id <id>    Set the Scanner's Hardware ID: <PAD-104>" << std::endl;
    std::cout << "  -p, --port <port>   Set the TCP port: <59153>" << std::endl;
    std::cout << "  -h, --help          Show this help message" << std::endl;
}

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
  std::string hwId = "PAD-104";
  int port = 59153;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      printUsage(argv[0]);
      return 0;
    } else if (arg == "-p" || arg == "--port") {
      if (i + 1 < argc) {
        port = std::stoi(argv[++i]);
      } else {
        std::cerr << "Error: " << arg << " requires a port number" << std::endl;
        return 1;
      }
    } else if (arg == "-i" || arg == "--hw-id") {
      if (i + 1 < argc) {
        hwId = argv[++i];
      } else {
        std::cerr << "Error: " << arg << " requires a hardware ID" << std::endl;
        return 1;
      }
    } else {
      // Assuming remaining arguments are positional HWID for backward compatibility
      hwId = arg;
    }
  }

  std::cout << "Connection HWID: " << hwId << std::endl;
  std::cout << "Connection Port: " << port << std::endl;

  std::cout.precision(std::numeric_limits<double>::max_digits10 - 1);

  markerDetection::PhoXiCam camera(hwId);
  camera.InitDevice(false);

  TcpServer server(port);

  if (!server.start()) {
    std::cerr << "Failed to start the server" << std::endl;
    return -1;
  }

  if (!server.acceptClient()) {
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
