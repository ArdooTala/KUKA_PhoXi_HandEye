#include "kuka_utils.h"
#include <Eigen/Geometry>
#include <pugixml.hpp>

namespace KukaUtils {

E6POS::E6POS(std::string &xml_msg) {
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_string(xml_msg.data());
  // Check if parse was successful

  pugi::xpath_node pos = doc.select_node("/StateSend/Position");

  x = pos.node().attribute("X").as_double();
  y = pos.node().attribute("Y").as_double();
  z = pos.node().attribute("Z").as_double();
  a = pos.node().attribute("A").as_double();
  b = pos.node().attribute("B").as_double();
  c = pos.node().attribute("C").as_double();
}

Eigen::Vector3d E6POS::t() const { return Eigen::Vector3d(x, y, z); }

Eigen::Matrix3d E6POS::r() const { return abc2matrix3d(a, b, c); }

E6POS::operator Eigen::Isometry3d() const {
  Eigen::Isometry3d tf;
  tf.linear() = r();
  tf.translation() = t();
  return tf;
}

Eigen::Matrix3d E6POS::abc2matrix3d(double a, double b, double c) const {
  Eigen::Quaterniond q =
      Eigen::AngleAxisd(a * DEG2RAD, Eigen::Vector3d::UnitZ()) *
      Eigen::AngleAxisd(b * DEG2RAD, Eigen::Vector3d::UnitY()) *
      Eigen::AngleAxisd(c * DEG2RAD, Eigen::Vector3d::UnitX());

  return q.matrix();
}

} // namespace KukaUtils
