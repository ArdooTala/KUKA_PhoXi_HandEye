#include "tcp_server.h"
#include <arpa/inet.h> // For inet_ntop()
#include <cstring>     // For memset, strlen
#include <iostream>
#include <netinet/in.h> // For sockaddr_in, INADDR_ANY
#include <string>
#include <sys/socket.h> // For socket(), bind(), listen(), accept()
#include <unistd.h>     // For close(), read()

TcpServer::TcpServer(int port)
    : port(port), server_fd(-1), is_running(false), client_socket(0),
      client_address() {}

TcpServer::~TcpServer() {
  if (is_running) {
    stop();
  }
}

bool TcpServer::start() {
  struct sockaddr_in address;
  int opt = 1;

  // 1. Create a socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    std::cerr << "Socket creation failed" << std::endl;
    return false;
  }

  // 2. Set socket options (SO_REUSEADDR)
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    std::cerr << "setsockopt failed" << std::endl;
    close(server_fd);
    return false;
  }

  // 3. Define the server address structure
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
  address.sin_port = htons(port);       // Use the port from the constructor

  // 4. Bind the socket to the address and port
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    std::cerr << "Bind failed" << std::endl;
    close(server_fd);
    return false;
  }

  // 5. Listen for incoming connections
  if (listen(server_fd, 1) < 0) { // 3 is the backlog queue size
    std::cerr << "Listen failed" << std::endl;
    close(server_fd);
    return false;
  }

  std::cout << "Server listening on port " << port << "..." << std::endl;
  is_running = true;
  return true;
}

// --- acceptClient() Method ---
// Waits for and accepts a new client connection.
// This is a blocking call.
// Returns the new client's socket file descriptor, or -1 on failure.
int TcpServer::acceptClient() {
  int new_socket;
  int addrlen = sizeof(client_address);

  // 6. Wait for and accept a client connection
  new_socket = accept(server_fd, (struct sockaddr *)&client_address,
                      (socklen_t *)&addrlen);

  if (new_socket < 0) {
    // Only print an error if the server is supposed to be running
    if (is_running) {
      std::cerr << "Accept failed" << std::endl;
    }
    return -1;
  }

  client_socket = new_socket;

  return new_socket;
}

bool TcpServer::sendMessage(const std::string &message) {
  if (send(client_socket, message.c_str(), message.length(), 0) < 0) {
    std::cerr << "Failed to send message" << std::endl;
    return false;
  }
  return true;
}

std::string TcpServer::receiveMessage(int buffer_size) {
  // Create a buffer to hold the incoming data
  char *buffer = new char[buffer_size];
  std::memset(buffer, 0, buffer_size); // Clear the buffer

  int bytes_read = read(client_socket, buffer, buffer_size - 1);

  if (bytes_read < 0) {
    std::cerr << "Read failed" << std::endl;
    delete[] buffer;
    return "";
  } else if (bytes_read == 0) {
    std::cout << "Client disconnected" << std::endl;
    delete[] buffer;
    stop();
    return ""; // Client closed the connection
  }

  // Convert the C-style char buffer to a C++ string
  std::string received_data(buffer);
  delete[] buffer; // Free the dynamically allocated buffer

  return received_data;
}

void TcpServer::stop() {
  if (client_socket) {
    close(client_socket);
    client_socket = -1;
  }
  if (is_running) {
    close(server_fd);
    server_fd = -1;
    is_running = false;
    std::cout << "Server shutting down." << std::endl;
  }
}

std::string TcpServer::getClientIp() {
  char client_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
  return std::string(client_ip);
}
