#define PHOXI_PCL_SUPPORT
#include "PhoXi.h"

#include "detector/marker_detector.h"
#include "kuka_utils/kuka_utils.h"
#include "pugixml.hpp"
#include "server/tcp_server.h"
#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <string>

#define PORT 59153

void convertToPCL(const pho::api::PFrame &Frame) {
  std::cout << "Frame " << Frame << "\n";
  pho::api::PhoXiTimeout timeout;
  pcl::PointCloud<pcl::PointXYZ>::Ptr PCLCloud(
      new pcl::PointCloud<pcl::PointXYZ>());

  pcl::PointCloud<pcl::PointXYZRGBNormal> MyPCLCloud;
  Frame->ConvertTo(MyPCLCloud);

  std::cout << "Number of points in PCL Cloud : " << MyPCLCloud.points.size()
            << std::endl;
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

  std::vector<Eigen::Isometry3d> rob2world;
  std::vector<Eigen::Isometry3d> cam2board;
  rob2world.reserve(6);
  cam2board.reserve(6);

  markerDetection::PhoXiCam camera(hwId);
  camera.InitDevice(true);

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
  while (!(msg = server.receiveMessage()).empty()) {
    std::cout << msg << std::endl;

    if (msg.find("<Position") == std::string::npos)
      break;
    auto rob_pos = KukaUtils::E6POS(msg);
    std::cout << ">>> Robot TCP Position:" << std::endl
              << ((Eigen::Isometry3d)rob_pos).matrix() << std::endl;

    try {
      camera.Trigger();
    } catch (...) {
      std::cerr << "Capture failed!...Or, the marker was not detected!!"
                << std::endl;
      server.sendMessage("<BasicRecv><Flag12></Flag12></BasicRecv>");
      continue;
    }

    server.sendMessage("<BasicRecv><Flag12></Flag12></BasicRecv>");
  }

  while (!server.receiveMessage().empty())
    ;

  return 0;
}
