#include "tcp_server.h"
#include <cstring>
#include <iostream>
#include <string>

#ifdef _WIN32
#define close closesocket
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

TcpServer::TcpServer(int port)
    : port(port), server_fd(-1), is_running(false), client_socket(0),
      client_address() {
#ifdef _WIN32
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed." << std::endl;
  }
#endif
}

TcpServer::~TcpServer() {
  if (is_running) {
    stop();
  }
#ifdef _WIN32
  WSACleanup();
#endif
}

bool TcpServer::start() {
  struct sockaddr_in address;
  int opt = 1;

  // 1. Create a socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
    std::cerr << "Socket creation failed" << std::endl;
    return false;
  }

#ifdef _WIN32
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt,
                 sizeof(opt)) == SOCKET_ERROR) {
#else
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)) < 0) {
#endif
    std::cerr << "setsockopt failed" << std::endl;
    close(server_fd);
    return false;
  }

  // 3. Define the server address structure
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
  address.sin_port = htons(port);       // Use the port from the constructor

  // 4. Bind the socket to the address and port
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) ==
      SOCKET_ERROR) {
    std::cerr << "Bind failed" << std::endl;
    close(server_fd);
    return false;
  }

  // 5. Listen for incoming connections
  if (listen(server_fd, 1) == SOCKET_ERROR) {
    std::cerr << "Listen failed" << std::endl;
    close(server_fd);
    return false;
  }

  std::cout << "Server listening on port " << port << "..." << std::endl;
  is_running = true;
  return true;
}

bool TcpServer::acceptClient() {
  SOCKET new_socket;
  int addrlen = sizeof(client_address);

  // 6. Wait for and accept a client connection
#ifdef _WIN32
  new_socket = accept(server_fd, (struct sockaddr *)&client_address, &addrlen);
#else
  new_socket = accept(server_fd, (struct sockaddr *)&client_address,
                      (socklen_t *)&addrlen);
#endif

  if (new_socket == INVALID_SOCKET) {
    if (is_running) {
      std::cerr << "Accept failed" << std::endl;
    }
    return false;
  }

  client_socket = new_socket;
  return true;
}

bool TcpServer::sendMessage(const std::string &message) {
  if (send(client_socket, message.c_str(), message.length(), 0) ==
      SOCKET_ERROR) {
    std::cerr << "Failed to send message" << std::endl;
    return false;
  }
  return true;
}

std::string TcpServer::receiveMessage(int buffer_size) {
  char *buffer = new char[buffer_size];
  std::memset(buffer, 0, buffer_size); // Clear the buffer

  int bytes_read = recv(client_socket, buffer, buffer_size - 1, 0);

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

  std::string received_data(buffer);
  delete[] buffer;
  return received_data;
}

void TcpServer::stop() {
  if (client_socket != INVALID_SOCKET) {
    close(client_socket);
    client_socket = INVALID_SOCKET;
  }
  if (is_running && server_fd != INVALID_SOCKET) {
    std::cout << "Server shutting down." << std::endl;
    close(server_fd);
    server_fd = INVALID_SOCKET;
    is_running = false;
  }
}

std::string TcpServer::getClientIp() {
  char client_ip[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
  return std::string(client_ip);
}
