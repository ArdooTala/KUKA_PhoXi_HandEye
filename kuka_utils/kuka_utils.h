#pragma once

#include <Eigen/Geometry>
#include <pugixml.hpp>

namespace KukaUtils {

const double DEG2RAD = M_PI / 180.0;
const double RAD2DEG = 180 / M_PI;

struct E6POS {
    double x, y, z, a, b, c;

    E6POS (std::string& xml_msg);
    E6POS (const Eigen::Isometry3d& frame);

    Eigen::Vector3d t() const;
    Eigen::Matrix3d r() const;
    Eigen::Isometry3d tf() const;
    operator Eigen::Isometry3d() const;
    operator Eigen::Isometry3f() const;

private:
    Eigen::Matrix3d abc2matrix3d(double a, double b, double c) const;
};


struct EKI_MSG {
    pugi::xml_document doc;
    pugi::xml_node root;
    EKI_MSG();

    void eki_add_flag ();
    void eki_add_frame (const E6POS& pos);
    void eki_add_message (const std::string& msg);

    std::string get_string();
};

} // namespace KukaUtils
