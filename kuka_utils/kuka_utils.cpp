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

Eigen::Isometry3d E6POS::tf() const {
  Eigen::Isometry3d tf;
  tf.linear() = r();
  tf.translation() = t();
  return tf;
}

E6POS::operator Eigen::Isometry3d() const { return tf(); }

Eigen::Matrix3d E6POS::abc2matrix3d(double a, double b, double c) const {
  Eigen::Quaterniond q =
      Eigen::AngleAxisd(a * DEG2RAD, Eigen::Vector3d::UnitZ()) *
      Eigen::AngleAxisd(b * DEG2RAD, Eigen::Vector3d::UnitY()) *
      Eigen::AngleAxisd(c * DEG2RAD, Eigen::Vector3d::UnitX());

  return q.matrix();
}

EKI_MSG::EKI_MSG() {
    root = doc.append_child("BasicRecv");
}

void EKI_MSG::eki_add_flag() {
    root.append_child("Flag12");
}

void EKI_MSG::eki_add_message (const std::string& msg) {
    root.append_child("Message").text().set(msg);
}

void EKI_MSG::eki_add_frame (Eigen::Isometry3d frame) {
    Eigen::Vector3d r = frame.rotation().eulerAngles(2, 1, 0);
    Eigen::Vector3d t = frame.translation();

    pugi::xml_node node = root.append_child("Frame");
    node.append_attribute("X") = t[0];
    node.append_attribute("Y") = t[1];
    node.append_attribute("Z") = t[2];
    node.append_attribute("A") = r[0];
    node.append_attribute("B") = r[1];
    node.append_attribute("C") = r[2];
}

std::string EKI_MSG::get_string() {
    std::ostringstream oss;
    doc.save(oss, "", pugi::format_raw);

    return oss.str();
}    
} // namespace KukaUtils
