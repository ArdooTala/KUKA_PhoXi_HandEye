#include "kuka_utils.h"
#include <Eigen/Dense>

namespace KukaUtils {

Eigen::Matrix3d eulerZYX_to_matrix3d(double a, double b, double c) {
  Eigen::Quaterniond q =
      Eigen::AngleAxisd(a * deg2rad, Eigen::Vector3d::UnitZ()) *
      Eigen::AngleAxisd(b * deg2rad, Eigen::Vector3d::UnitY()) *
      Eigen::AngleAxisd(c * deg2rad, Eigen::Vector3d::UnitX());

  return q.matrix();
}
} // namespace KukaUtils
