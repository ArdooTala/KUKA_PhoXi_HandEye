#pragma once

#include <Eigen/Geometry>
#include <pugixml.hpp>

namespace KukaUtils {

const double DEG2RAD = M_PI / 180.0;

struct E6POS {
    double x, y, z, a, b, c;

    E6POS (std::string& xml_msg);

    Eigen::Vector3d t() const;
    Eigen::Matrix3d r() const;
    operator Eigen::Transform<double, 3, 1>() const;

private:
    Eigen::Matrix3d abc2matrix3d(double a, double b, double c) const;
};

} // namespace KukaUtils
