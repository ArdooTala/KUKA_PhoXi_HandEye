#include "detector/marker_detector.h"
#include "server/tcp_server.h"
#include <iostream>
#include <string>

#define PORT 59153

int main() {
  std::cout << "Hi there lion...!" << std::endl;

  markerDetection::PhoXiCam camera("PAD-104");
  camera.initDevice();

  TcpServer server(PORT);
  server.msgCallback = [&camera](std::string msg) {
    std::cout << "Callbacking...!!" << std::endl;
    camera.trigger();
    auto tf = camera.getCameraTransform();
    std::cout << "x: " << tf.Translation.x << std::endl;
    std::cout << "y: " << tf.Translation.y << std::endl;
    std::cout << "z: " << tf.Translation.z << std::endl;
    std::cout << msg << std::endl;
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
  }

  return 0;
}
