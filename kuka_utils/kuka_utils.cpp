#include "kuka_utils.h"
#include <Eigen/Dense>
#include <pugixml.hpp>

namespace KukaUtils {

Eigen::Matrix<double, 6, 1> e6pos_from_xml(std::string &xmlMsg) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string(xmlMsg.data());
    // Check if parse was successful

    pugi::xpath_node pos = doc.select_node("/StateSend/Position");

    double x = pos.node().attribute("X").as_double();
    double y = pos.node().attribute("Y").as_double();
    double z = pos.node().attribute("Z").as_double();
    double a = pos.node().attribute("A").as_double();
    double b = pos.node().attribute("B").as_double();
    double c = pos.node().attribute("C").as_double();
    Eigen::Matrix<double, 6, 1> e6pos;
    e6pos << x, y, z, a, b, c;

    return e6pos;
}

Eigen::Matrix3d eulerZYX_to_matrix3d(double a, double b, double c) {
  Eigen::Quaterniond q =
      Eigen::AngleAxisd(a * deg2rad, Eigen::Vector3d::UnitZ()) *
      Eigen::AngleAxisd(b * deg2rad, Eigen::Vector3d::UnitY()) *
      Eigen::AngleAxisd(c * deg2rad, Eigen::Vector3d::UnitX());

  return q.matrix();
}

} // namespace KukaUtils
