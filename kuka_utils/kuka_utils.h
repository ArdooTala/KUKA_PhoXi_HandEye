#pragma once

#include <Eigen/Geometry>

namespace KukaUtils {

const double deg2rad = M_PI / 180.0;

Eigen::Matrix<double, 6, 1> e6pos_from_xml(std::string &xmlMsg);

Eigen::Matrix3d eulerZYX_to_matrix3d(double a, double b, double c);

} // namespace KukaUtils
