#include "detector/marker_detector.h"
#include "handeye/handeye.h"
#include "kuka_utils/kuka_utils.h"
#include "pugixml.hpp"
#include "server/tcp_server.h"
#include <iostream>
#include <string>

#define PORT 59153

std::vector<Eigen::Transform<double, 3, 1>> rob2world(20);
std::vector<Eigen::Transform<double, 3, 1>> cam2board(20);

int main() {
  std::cout.precision(std::numeric_limits<double>::max_digits10 - 1);
  markerDetection::PhoXiCam camera("PAD-104");
  camera.initDevice();

  TcpServer server(PORT);
  server.msgCallback = [&camera](std::string msg) {
    std::cout << "Callbacking...!!" << std::endl;
    std::cout << msg << std::endl;

    auto rob_pos = KukaUtils::E6POS(msg);
    std::cout << rob_pos.t() << std::endl;
    std::cout << rob_pos.r() << std::endl;

    try {
      camera.trigger();
      auto tf = camera.getCameraTransform();
      std::cout << "x: " << tf.Translation.x << std::endl;
      std::cout << "y: " << tf.Translation.y << std::endl;
      std::cout << "z: " << tf.Translation.z << std::endl;

      auto marker_tf = markerDetection::phoxi2eigen(tf);
      std::cout << marker_tf.translation() << std::endl;
      std::cout << marker_tf.rotation() << std::endl;

    } catch (...) {
      std::cerr << "Capture failed!" << std::endl;
    }

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
