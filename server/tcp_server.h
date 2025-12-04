#pragma once

#include <functional>
#include <netinet/in.h> // For sockaddr_in
#include <string>

/**
 * @class TcpServer
 * @brief A simple C++ wrapper for a POSIX TCP server.
 *
 * This class encapsulates the logic for creating, binding, listening,
 * and accepting connections on a TCP socket.
 */
class TcpServer {
public:
  /**
   * @brief Constructor: Initializes the server with a specific port.
   * @param port The port number to listen on.
   */
  TcpServer(int port);

  /**
   * @brief Destructor: Cleans up resources, stops the server if running.
   */
  ~TcpServer();

  /**
   * @brief Starts the server: creates socket, binds, and listens.
   * @return true on success, false on failure.
   */
  bool start();

  /**
   * @brief Waits for and accepts a new client connection (blocking).
   * @param[out] client_address A sockaddr_in structure that will be filled
   * with the connecting client's address info.
   * @return The client's socket file descriptor, or -1 on failure.
   */
  int acceptClient();

  /**
   * @brief Sends a message to a specific client.
   * @param client_socket The file descriptor for the client.
   * @param message The string message to send.
   * @return true on success, false on failure.
   */
  bool sendMessage(const std::string &message);

  /**
   * @brief Receives a message from a specific client (blocking).
   * @param client_socket The file descriptor for the client.
   * @param buffer_size The (optional) maximum amount of data to read at once.
   * @return A string containing the received data. Empty on error or
   * disconnect.
   */
  std::string receiveMessage(int buffer_size = 1024);

  /**
   * @brief Closes the server's listening socket and stops the server.
   */
  void stop();

  /**
   * @brief A static helper function to get a client's IP as a string.
   * @param client_address The client's address structure.
   * @return A string representation of the client's IP address.
   */
  std::string getClientIp();

  std::function<void(const std::string &)> msgCallback;

private:
  int port;        // Port number for the server
  int server_fd;   // Server's listening socket file descriptor
  bool is_running; // Flag to indicate if the server is active

  int client_socket;
  struct sockaddr_in client_address;
};
