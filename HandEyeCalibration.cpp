#include "detector/marker_detector.h"
#include "handeye/handeye.h"
#include "kuka_utils/kuka_utils.h"
#include "pugixml.hpp"
#include "server/tcp_server.h"
#include <iostream>
#include <string>

#define PORT 59153

std::vector<Eigen::Isometry3d> rob2world;
std::vector<Eigen::Isometry3d> cam2board;

int main() {
  std::cout.precision(std::numeric_limits<double>::max_digits10 - 1);
  rob2world.reserve(20);
  cam2board.reserve(20);

  markerDetection::PhoXiCam camera("PAD-104");
  camera.initDevice();

  TcpServer server(PORT);
  server.msgCallback = [&camera](std::string msg) {
    std::cout << "Callbacking...!!" << std::endl;

    try {
      camera.trigger();
    } catch (...) {
      std::cerr << "Capture failed!" << std::endl;
      return;
    }

    auto rob_pos = KukaUtils::E6POS(msg);
    std::cout << ">>> Robot TCP Position:" << std::endl
              << ((Eigen::Isometry3d)rob_pos).matrix() << std::endl;

    auto tf = camera.getCameraTransform();
    auto marker_tf = markerDetection::phoxi2eigen(tf);
    std::cout << ">>> Camera Position in Marker Coordinate Space:" << std::endl
              << marker_tf.matrix() << std::endl;

    cam2board.push_back(marker_tf);
    rob2world.push_back(rob_pos);
  };

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

  while (!server.receiveMessage().empty()) {
    server.sendMessage("<BasicRecv><Flag12></Flag12></BasicRecv>");
  }

  return 0;
}
