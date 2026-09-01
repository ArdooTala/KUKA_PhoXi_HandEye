#include "handeye.h"
#include <iostream>
#include <opencv2/calib3d.hpp>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Dense>

#include <opencv2/core.hpp>
#include <opencv2/core/eigen.hpp>

namespace HandEye {

HandEye::HandEye(std::vector<Eigen::Isometry3d> &cam,
                 std::vector<Eigen::Isometry3d> &rob)
    : m_cam(cam), m_rob(rob), m_size(cam.size()) {
  std::cout << "Array Size: " << m_size << std::endl;
}

Eigen::Isometry3d HandEye::calculate_handeye() {
  std::vector<cv::Mat> R_gripper2base(m_size), t_gripper2base(m_size),
      R_target2cam(m_size), t_target2cam(m_size);
  cv::Mat t_cam2gripper, R_cam2gripper;

  for (int i = 0; i < m_size; i++) {
    cv::eigen2cv((Eigen::Matrix3d)m_rob[i].linear(), R_gripper2base[i]);
    cv::eigen2cv((Eigen::Vector3d)m_rob[i].translation(), t_gripper2base[i]);
    cv::eigen2cv((Eigen::Matrix3d)m_cam[i].linear(), R_target2cam[i]);
    cv::eigen2cv((Eigen::Vector3d)m_cam[i].translation(), t_target2cam[i]);

    std::cout << "tvecs" << std::endl;
    std::cout << "ROB" << std::endl;
    std::cout << t_gripper2base[i] << std::endl;
    std::cout << m_rob[i].translation() << std::endl;
    std::cout << "CAM" << std::endl;
    std::cout << t_target2cam[i] << std::endl;
    std::cout << m_cam[i].translation() << std::endl;
  }

  cv::calibrateHandEye(R_gripper2base, t_gripper2base, R_target2cam,
                       t_target2cam, R_cam2gripper, t_cam2gripper);
  // cv::CALIB_HAND_EYE_DANIILIDIS);

  Eigen::Matrix3d R_res;
  cv::cv2eigen(R_cam2gripper, R_res);
  Eigen::Vector3d t_res;
  cv::cv2eigen(t_cam2gripper, t_res);
  gripper2cam.translation() = t_res;
  gripper2cam.linear() = R_res;

  return gripper2cam;
}

Eigen::Isometry3d HandEye::estimate_board_pose() {
    world2target.clear();

    Eigen::Vector3d mean = Eigen::Vector3d::Zero();
    for (int i = 0 ; i < m_size ; i++) {
        auto prj = reproject(m_rob[i], m_cam[i]);
        world2target.push_back(prj);
        mean += prj.translation();
    }
    mean /= static_cast<double>(m_size);

    Eigen::Isometry3d board_pose = Eigen::Isometry3d::Identity();
    board_pose.translation() = mean;

    if (m_size > 0) {
        Eigen::Quaterniond q_acc(world2target[0].linear());
        for (int i = 1 ; i < m_size ; i++) {
            Eigen::Quaterniond q(world2target[i].linear());
            if (q_acc.dot(q) < 0.0)
                q.coeffs() *= -1.0;
            q_acc.coeffs() += q.coeffs();
        }
        q_acc.normalize();
        board_pose.linear() = q_acc.toRotationMatrix();
    }

    return board_pose;
}

Eigen::Isometry3d HandEye::reproject(Eigen::Isometry3d rob_tf, Eigen::Isometry3d cam_tf) {
    return rob_tf * gripper2cam * cam_tf;
}

} // namespace HandEye
