#pragma once

#include <Eigen/Geometry>
#include <opencv2/core/mat.hpp>

namespace HandEye {

Eigen::Transform<double, 3, 1>
calibrate_hand_eye(std::vector<Eigen::Transform<double, 3, 1>>& cam,
                   std::vector<Eigen::Transform<double, 3, 1>>& rob);

} // namespace HandEye
