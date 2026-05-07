#pragma once

#include <PhoXi.h>
#include <Eigen/Geometry>
#include <string>

namespace markerDetection {

class PhoXiCam {
public:
  PhoXiCam(std::string id);
  ~PhoXiCam();
  void InitDevice(bool calibration = true);
  void Trigger();
  pho::api::PhoXiCoordinateTransformation GetCameraTransform();
  void SaveFrame();

private:
  bool Connect();

private:
  std::string hwId;
  pho::api::PhoXiFactory factory;
  pho::api::PPhoXi device;
  pho::api::PFrame frame;
  int lastFrameIndex;
  pho::api::PhoXiCoordinateTransformation cameraPosition;
};

Eigen::Isometry3d Phoxi2Eigen (pho::api::PhoXiCoordinateTransformation& phoxi_tf) ;

} // namespace markerDetection
