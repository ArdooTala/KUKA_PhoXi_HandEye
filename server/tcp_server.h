#pragma once

#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
#endif

class TcpServer {
public:
  TcpServer(int port);

  ~TcpServer();

  bool start();

  bool acceptClient();

  bool sendMessage(const std::string &message);

  std::string receiveMessage(int buffer_size = 1024);

  void stop();

  std::string getClientIp();

private:
  int port;
  SOCKET server_fd;
  bool is_running;

  SOCKET client_socket;
  struct sockaddr_in client_address;
};
