#pragma once

#include <PhoXi.h>
#include <Eigen/Geometry>
#include <string>

namespace markerDetection {

class PhoXiCam {
public:
  PhoXiCam(std::string id);
  ~PhoXiCam();
  void initDevice();
  void trigger();
  pho::api::PhoXiCoordinateTransformation getCameraTransform();
  void saveFrame();

private:
  bool connect();

private:
  std::string hwId;
  pho::api::PhoXiFactory factory;
  pho::api::PPhoXi device;
  pho::api::PFrame frame;
  int lastFrameIndex;
  pho::api::PhoXiCoordinateTransformation cameraPosition;
};

Eigen::Isometry3d phoxi2eigen (pho::api::PhoXiCoordinateTransformation& phoxi_tf) ;

} // namespace markerDetection
