#pragma once

#include <Eigen/Geometry>
#include <opencv2/core/mat.hpp>

namespace HandEye {

class HandEye {
public:
  HandEye(std::vector<Eigen::Isometry3d> &cam,
          std::vector<Eigen::Isometry3d> &rob);

  Eigen::Isometry3d calculate_handeye();
  std::pair<Eigen::Vector3d, Eigen::Matrix3d> calculate_reprojection();

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
