#pragma once

#include <Eigen/Geometry>
#include <opencv2/core/mat.hpp>

namespace HandEye {

struct ReprojectionError {
  double mean_t, max_t, var_t;
  double mean_r, max_r, var_r;
};

class HandEye {
public:
  HandEye(std::vector<Eigen::Isometry3d> &cam,
          std::vector<Eigen::Isometry3d> &rob);

  Eigen::Isometry3d calculate_handeye(int method = 0);
  Eigen::Isometry3d estimate_board_pose();
  ReprojectionError calculate_reprojection_error();

private:
  Eigen::Isometry3d reproject(Eigen::Isometry3d rob_tf,
                              Eigen::Isometry3d cam_tf);

  std::vector<Eigen::Isometry3d> &m_cam;
  std::vector<Eigen::Isometry3d> &m_rob;
  std::vector<Eigen::Isometry3d> world2target;
  Eigen::Isometry3d gripper2cam;
  int m_size;
};

} // namespace HandEye
