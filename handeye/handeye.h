#pragma once

#include <Eigen/Geometry>
#include <opencv2/core/mat.hpp>

namespace HandEye {

Eigen::Isometry3d
calibrate_hand_eye(std::vector<Eigen::Isometry3d>& cam,
                   std::vector<Eigen::Isometry3d>& rob);

} // namespace HandEye
