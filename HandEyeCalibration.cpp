#include "detector/marker_detector.h"
#include "handeye/handeye.h"
#include "kuka_utils/kuka_utils.h"
#include "pugixml.hpp"
#include "server/tcp_server.h"
#include <iostream>
#include <string>

#define PORT 59153

int main(int argc, char *argv[]) {
  std::string hwId;

  if (argc > 1) {
    std::cout << "Connection HWID: " << argv[1] << std::endl;
    hwId = argv[1];

    if (hwId.empty()) {
      std::cout << "Error reading the HardwareID" << std::endl;
      return 1;
    }
  } else {
    hwId = "PAD-104";
  }
  std::cout.precision(std::numeric_limits<double>::max_digits10 - 1);

  std::vector<Eigen::Isometry3d> rob2world;
  std::vector<Eigen::Isometry3d> cam2board;
  rob2world.reserve(6);
  cam2board.reserve(6);

  markerDetection::PhoXiCam camera(hwId);
  camera.InitDevice(true);

  TcpServer server(PORT);

  if (!server.start()) {
    std::cerr << "Failed to start the server" << std::endl;
    return -1;
  }

  if (!server.acceptClient()) {
    std::cerr << "Failed to accept client." << std::endl;
    server.stop();
    return -1;
  }

  // A client has connected!
  std::cout << "Client connected!" << std::endl;
  std::cout << "Client IP: " << server.getClientIp() << std::endl;

  std::string msg;
  while (!(msg = server.receiveMessage()).empty()) {
    std::cout << msg << std::endl;

    if (msg.find("<Position") == std::string::npos)
      break;
    auto rob_pos = KukaUtils::E6POS(msg);
    std::cout << ">>> Robot TCP Position:" << std::endl
              << ((Eigen::Isometry3d)rob_pos).matrix() << std::endl;

    try {
      camera.Trigger();
    } catch (...) {
      std::cerr << "Capture failed!...Or, the marker was not detected!!" << std::endl;
      server.sendMessage("<BasicRecv><Flag12></Flag12></BasicRecv>");
      continue;
    }

    auto tf = camera.GetCameraTransform();
    auto marker_tf = markerDetection::Phoxi2Eigen(tf);
    std::cout << ">>> Camera Position in Marker Coordinate Space:" << std::endl
              << marker_tf.matrix() << std::endl;

    rob2world.push_back(rob_pos);
    cam2board.push_back(marker_tf.inverse());

    server.sendMessage("<BasicRecv><Flag12></Flag12></BasicRecv>");
  }

  auto he = HandEye::HandEye(cam2board, rob2world);

  auto res = he.calculate_handeye();
  std::cout << ">>> RESULT <<<" << std::endl << res.matrix() << std::endl;

  if (msg.find("<Tool") != std::string::npos) {
    std::cout << "Sending Tool Data" << std::endl;
    KukaUtils::EKI_MSG eki_msg;
    eki_msg.eki_add_message("MotionCam_M");
    eki_msg.eki_add_frame(res);

    server.sendMessage(eki_msg.get_string());
  }

  auto reproj_error = he.calculate_reprojection();
  std::cout << ">>> ERROR <<<" << std::endl << reproj_error.first << std::endl;

  while (!server.receiveMessage().empty())
    ;

  return 0;
}
