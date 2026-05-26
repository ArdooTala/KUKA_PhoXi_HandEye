#include "tcp_server.h"
#include <iostream>
#include <vector>

TcpServer::TcpServer(int port)
    : port(port), io_context(), acceptor(io_context), socket(io_context),
      is_running(false) {}

TcpServer::~TcpServer() { stop(); }

bool TcpServer::start() {
  try {
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();

    std::cout << "Server listening on port " << port << "..." << std::endl;
    is_running = true;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Start failed: " << e.what() << std::endl;
    return false;
  }
}

bool TcpServer::acceptClient() {
  try {
    acceptor.accept(socket);
    return true;
  } catch (const std::exception &e) {
    if (is_running) {
      std::cerr << "Accept failed: " << e.what() << std::endl;
    }
    return false;
  }
}

bool TcpServer::sendMessage(const std::string &message) {
  try {
    asio::write(socket, asio::buffer(message));
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Failed to send message: " << e.what() << std::endl;
    return false;
  }
}

std::string TcpServer::receiveMessage(int buffer_size) {
  try {
    std::vector<char> buffer(buffer_size);
    asio::error_code error;
    size_t length = socket.receive(asio::buffer(buffer), 0, error);

    if (error == asio::error::eof) {
      std::cout << "Client disconnected" << std::endl;
      stop();
      return "";
    } else if (error) {
      throw asio::system_error(error);
    }

    return std::string(buffer.data(), length);
  } catch (const std::exception &e) {
    std::cerr << "Read failed: " << e.what() << std::endl;
    return "";
  }
}

void TcpServer::stop() {
  is_running = false;
  asio::error_code ec;
  socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
  socket.close(ec);
  acceptor.close(ec);
}

std::string TcpServer::getClientIp() {
  try {
    return socket.remote_endpoint().address().to_string();
  } catch (const std::exception &e) {
    return "";
  }
}
