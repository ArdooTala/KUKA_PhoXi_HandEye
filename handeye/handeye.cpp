#include "handeye.h"
#include <opencv2/calib3d.hpp>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>
    
#include <opencv2/core.hpp>
#include <opencv2/core/eigen.hpp>

namespace HandEye {

Eigen::Transform<double, 3, 1>
calibrate_hand_eye(std::vector<Eigen::Transform<double, 3, 1>> &cam,
                   std::vector<Eigen::Transform<double, 3, 1>> &rob) {
  int count = cam.size();
  std::vector<cv::Mat> R_gripper2base(count), t_gripper2base(count),
      R_target2cam(count), t_target2cam(count);

  for (int i = 0; i < cam.size(); i++) {
    cv::eigen2cv((Eigen::Matrix3d)rob[i].rotation(), R_gripper2base[i]);
    cv::eigen2cv((Eigen::Vector3d)rob[i].translation(), t_gripper2base[i]);
    cv::eigen2cv((Eigen::Matrix3d)cam[i].rotation(), R_target2cam[i]);
    cv::eigen2cv((Eigen::Vector3d)cam[i].translation(), t_target2cam[i]);
  }

  cv::Mat t_cam2gripper, R_cam2gripper;

  cv::calibrateHandEye(R_gripper2base, t_gripper2base, R_target2cam,
                       t_target2cam, R_cam2gripper, t_cam2gripper,
                       cv::CALIB_HAND_EYE_DANIILIDIS);

  Eigen::Matrix3d R_res;
  cv::cv2eigen(R_cam2gripper, R_res);

  Eigen::Vector3d t_res;
  cv::cv2eigen(t_cam2gripper, t_res);

  Eigen::Transform<double, 3, 1> result;
  result.translate(t_res);
  result.rotate(R_res);

  return result;
}

} // namespace HandEye
