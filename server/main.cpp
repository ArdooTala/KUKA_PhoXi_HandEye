#include "tcp_server.h"
#include <iostream>
#include <string>
#include <unistd.h> // For close()

#define PORT 12121

void handleXmlMessage(const std::string &msg) {
  std::cout << "[CALLBACK] Received a message: " << msg << std::endl;
}

int main() {
  TcpServer server(PORT);

  if (!server.start()) {
    std::cerr << "Failed to start the server." << std::endl;
    return -1;
  }

  server.msgCallback = handleXmlMessage;

  if (server.acceptClient() < 0) {
    std::cerr << "Failed to accept client." << std::endl;
    server.stop();
    return -1;
  }

  // A client has connected!
  std::cout << "Client connected!" << std::endl;
  std::cout << "Client IP: " << server.getClientIp() << std::endl;

  // 4. Send a welcome message to the client
  server.sendMessage("Hello from the server library!");

  // 5. Read a message from the client
  std::string client_msg = server.receiveMessage();
  if (!client_msg.empty()) {
    std::cout << "Client said: " << client_msg << std::endl;
  }

  // 6. Close the client connection and stop the server
  server.stop();

  return 0;
}
