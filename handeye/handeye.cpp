#include "handeye.h"
#include <iostream>
#include <opencv2/calib3d.hpp>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>

#include <opencv2/core.hpp>
#include <opencv2/core/eigen.hpp>

namespace HandEye {

Eigen::Isometry3d calibrate_hand_eye(std::vector<Eigen::Isometry3d> &cam,
                                     std::vector<Eigen::Isometry3d> &rob) {
  int count = cam.size();
  std::cout << "Array Size: " << count << std::endl;

  std::vector<cv::Mat> R_gripper2base(count), t_gripper2base(count),
      R_target2cam(count), t_target2cam(count);

  for (int i = 0; i < cam.size(); i++) {
    cv::eigen2cv((Eigen::Matrix3d)rob[i].linear(), R_gripper2base[i]);
    cv::eigen2cv((Eigen::Vector3d)rob[i].translation(), t_gripper2base[i]);
    cv::eigen2cv((Eigen::Matrix3d)cam[i].linear(), R_target2cam[i]);
    cv::eigen2cv((Eigen::Vector3d)cam[i].translation(), t_target2cam[i]);

    std::cout << "tvecs" << std::endl;
    std::cout << "ROB" << std::endl;
    std::cout << t_gripper2base[i] << std::endl;
    std::cout << rob[i].translation() << std::endl;
    std::cout << "CAM" << std::endl;
    std::cout << t_target2cam[i] << std::endl;
    std::cout << cam[i].translation() << std::endl;
  }

  cv::Mat t_cam2gripper, R_cam2gripper;

  cv::calibrateHandEye(R_gripper2base, t_gripper2base, R_target2cam,
                       t_target2cam, R_cam2gripper, t_cam2gripper);
                       // cv::CALIB_HAND_EYE_DANIILIDIS);

  Eigen::Matrix3d R_res;
  cv::cv2eigen(R_cam2gripper, R_res);

  Eigen::Vector3d t_res;
  cv::cv2eigen(t_cam2gripper, t_res);

  Eigen::Isometry3d result;
  result.translation() = t_res;
  result.linear() = R_res;

  return result;
}

} // namespace HandEye
