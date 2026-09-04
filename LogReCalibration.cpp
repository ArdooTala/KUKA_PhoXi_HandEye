#include "handeye/handeye.h"
#include "kuka_utils/kuka_utils.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#define PORT 59153

namespace {

enum class BlockType { None, Robot, Camera };

bool parse_matrix_row(std::istringstream &iss, Eigen::Matrix4d &m, int row) {
  if (row < 3) {
    if (!(iss >> m(row, 0) >> m(row, 1) >> m(row, 2) >> m(row, 3)))
      return false;
  } else {
    if (!(iss >> m(3, 0) >> m(3, 1) >> m(3, 2) >> m(3, 3)))
      return false;
  }
  return true;
}

} // namespace

int main(int argc, char *argv[]) {
  std::string logFile;

  if (argc < 2) {
    std::cout << "Missing argument: Please provide the path to the log file"
              << std::endl;
    return 1;
  }
  logFile = argv[1];

  if (logFile.empty()) {
    std::cout << "Error reading the Log file path" << std::endl;
    return 1;
  }

  int method = 0;
  if (argc > 2) {
    try {
      method = std::stoi(argv[2]);
    } catch (const std::invalid_argument &e) {
      std::cerr << "Invalid method argument: " << argv[2]
          << " (must be an integer)" << std::endl;
      return 1;
    }
  }


  std::cout.precision(std::numeric_limits<double>::max_digits10 - 1);

  std::vector<Eigen::Isometry3d> rob2world, cam2board;

  std::ifstream file(logFile);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot open log file: " << logFile << std::endl;
    return 1;
  }

  BlockType block = BlockType::None;
  Eigen::Matrix4d mat = Eigen::Matrix4d::Identity();
  int row = 0;

  std::string line;
  while (std::getline(file, line)) {
    if (line.find(">>> Robot TCP Position:") != std::string::npos) {
      block = BlockType::Robot;
      row = 0;
      mat = Eigen::Matrix4d::Identity();
    } else if (line.find(">>> Camera Position in Marker Coordinate Space:") !=
               std::string::npos) {
      block = BlockType::Camera;
      row = 0;
      mat = Eigen::Matrix4d::Identity();
    } else if (block != BlockType::None) {
      std::istringstream iss(line);
      double probe;
      if (!(iss >> probe))
        continue;

      iss.clear();
      iss.seekg(0);
      if (!parse_matrix_row(iss, mat, row)) {
        std::cerr << "Error: Failed to parse transformation matrix row: "
                  << line << std::endl;
        return 1;
      }

      if (++row == 4) {
        Eigen::Isometry3d tf;
        tf.linear() = mat.block<3, 3>(0, 0);
        tf.translation() = mat.block<3, 1>(0, 3);

        if (block == BlockType::Robot)
          rob2world.push_back(tf);
        else
          cam2board.push_back(tf.inverse());

        block = BlockType::None;
      }
    }
  }

  if (rob2world.empty() || rob2world.size() != cam2board.size()) {
    std::cerr << "Error: Log parsed " << rob2world.size() << " robot poses but "
              << cam2board.size() << " camera poses" << std::endl;
    return 1;
  }

  auto he = HandEye::HandEye(cam2board, rob2world);

  auto res = he.calculate_handeye(method);
  std::cout << ">>> RESULT <<<" << std::endl << res.matrix() << std::endl;
  
  auto tool_pose = KukaUtils::E6POS(res);
  std::cout << ">>> Tool Pose <<<" << std::endl
            << "{X " << tool_pose.x
            << " Y " << tool_pose.y
            << " Z " << tool_pose.z
            << " A " << tool_pose.a
            << " B " << tool_pose.b
            << " C " << tool_pose.c
            << "}" << std::endl;

  auto board_pose = he.estimate_board_pose();
  std::cout << ">>> Board Pose <<<" << std::endl
            << board_pose.matrix() << std::endl;

  auto error = he.calculate_reprojection_error();
  std::cout << ">>> Error <<<" << std::endl;
  std::cout << "Mean     >> t: " << error.mean_t << " / r: " << error.mean_r
            << std::endl;
  std::cout << "Max      >> t: " << error.max_t << " / r: " << error.max_r
            << std::endl;
  std::cout << "Variance >> t: " << error.var_t << " / r: " << error.var_r
            << std::endl;

  return 0;
}
