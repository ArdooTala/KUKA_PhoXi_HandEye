#pragma once

#include <string>
#include <asio.hpp>

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
  asio::io_context io_context;
  asio::ip::tcp::acceptor acceptor;
  asio::ip::tcp::socket socket;
  bool is_running;
};
