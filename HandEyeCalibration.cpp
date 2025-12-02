#include "server/tcp_server.h"
#include "detector/marker_detector.h"
#include <iostream>
#include <string>

#define PORT 12321

int main() {
  std::cout << "Hi there lion...!" << std::endl;

  markerDetection::PhoXiCam camera("PAD-104");
  camera.initDevice();

  TcpServer server(PORT);
  server.msgCallback = [&camera](std::string msg) {
    std::cout << "Callbacking...!!" << std::endl;
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
  
    while (!server.receiveMessage().empty()) {}

  return 0;
}
