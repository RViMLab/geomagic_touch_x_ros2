#include <eigen3/Eigen/Dense>
#include <memory>
#include <stdexcept>
#include <string>

#include "geometry_msgs/msg/transform_stamped.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "geometry_msgs/msg/wrench.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "tf2_ros/transform_broadcaster.h"

#include "geomagic_touch_x/device.hpp"

namespace geomagic_touch_x {
class DeviceDriver : public rclcpp::Node {
protected:
  static constexpr double MM2M = 1.e-3;
  static constexpr uint8_t DOF = 6;

public:
  struct DriverParams {
    int update_rate;
    std::string frame_id;
    std::string child_frame_id;
    std::string device_name;
  };

  struct DeviceState {
    sensor_msgs::msg::JointState joint_state;
    geometry_msgs::msg::TwistStamped twist_stamped;
  };

public:
  DeviceDriver(const rclcpp::NodeOptions &options)
      : Node("geomagic_touch_x", options), q_prev_set_(false) {
    this->declare_params_();
    this->get_params_();
    this->init_msgs_();
    this->init_pubs_(); // cheers ;)
    this->init_subs_();
    ctrl_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(static_cast<int64_t>(1.e3 / params_.update_rate)),
        std::bind(&DeviceDriver::on_update_, this));
    RCLCPP_INFO(this->get_logger(), "Instantiating device %s...", params_.device_name.c_str());
    device_ = std::make_unique<Device>(params_.device_name);
    RCLCPP_INFO(this->get_logger(), "Done.");
  };

protected:
  // Device
  std::unique_ptr<Device> device_;

  // Published states
  geometry_msgs::msg::TransformStamped tf_stamped_;
  sensor_msgs::msg::JointState js_;
  bool q_prev_set_;
  Eigen::Matrix<double, DOF, 1> q_, q_prev_, dq_;
  geometry_msgs::msg::TwistStamped ts_;

  // Parameters
  DriverParams params_;

  // Device state
  DeviceState state_;

  // Publishers
  rclcpp::TimerBase::SharedPtr ctrl_timer_;
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr js_pub_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr ts_pub_;
  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_bc_;

  // Subscribers
  rclcpp::Subscription<geometry_msgs::msg::Wrench>::SharedPtr w_sub_;

protected:
  void declare_params_() {
    this->declare_parameter("update_rate", 200);
    this->declare_parameter("frame_id", "touch_x_base");
    this->declare_parameter("child_frame_id", "touch_x_ee");
    this->declare_parameter("device_name", "");
  };
  void get_params_() {
    this->get_parameter("update_rate", params_.update_rate);
    this->get_parameter("frame_id", params_.frame_id);
    this->get_parameter("child_frame_id", params_.child_frame_id);
    this->get_parameter("device_name", params_.device_name);
    if (params_.device_name.empty()) {
      std::string err = "No device_name given, shutting down.";
      RCLCPP_ERROR(this->get_logger(), err.c_str());
      throw std::runtime_error(err);
    }
    RCLCPP_INFO(this->get_logger(), "*** Parameters");
    RCLCPP_INFO(this->get_logger(), "*   update_rate: %i Hz", params_.update_rate);
    RCLCPP_INFO(this->get_logger(), "*   frame_id: %s", params_.frame_id.c_str());
    RCLCPP_INFO(this->get_logger(), "*   child_frame_id: %s", params_.child_frame_id.c_str());
  };
  void init_msgs_() {
    // Joint states
    js_.effort.resize(DOF, 0.);
    js_.position.resize(DOF, 0.);
    js_.velocity.resize(DOF, 0.);
    js_.name = {"joint_angle_1",  "joint_angle_2",  "joint_angle_3",
                "gimbal_angle_1", "gimbal_angle_2", "gimbal_angle_3"};
  }
  void init_pubs_() {
    js_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("~/joint_states",
                                                                   rclcpp::SensorDataQoS());
    ts_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>("~/twist",
                                                                       rclcpp::SensorDataQoS());
    tf_bc_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
  };
  void init_subs_() {
    w_sub_ = this->create_subscription<geometry_msgs::msg::Wrench>(
        "~/command/wrench", rclcpp::SensorDataQoS(),
        [this](geometry_msgs::msg::Wrench::SharedPtr msg) {
          auto &state = device_->get_state();
          state.cmd_force[0] = msg->force.x;
          state.cmd_force[1] = msg->force.y;
          state.cmd_force[2] = msg->force.z;
        });
  };
  void on_update_() {
    // get the current state
    const auto &state = device_->get_state();
    const auto stamp = this->get_clock()->now();

    // Pack transform stamp and translation
    tf_stamped_.header.stamp = stamp;
    tf_stamped_.header.frame_id = params_.frame_id;
    tf_stamped_.child_frame_id = params_.child_frame_id;
    tf_stamped_.transform.translation.x = MM2M * state.transform[12];
    tf_stamped_.transform.translation.y = MM2M * state.transform[13];
    tf_stamped_.transform.translation.z = MM2M * state.transform[14];

    // Convert state transform to rotation matrix, then quaternion, and pack transform
    Eigen::Matrix3d R(3, 3);
    R(0, 0) = state.transform[0];
    R(0, 1) = state.transform[1];
    R(0, 2) = state.transform[2];
    R(1, 0) = state.transform[4];
    R(1, 1) = state.transform[5];
    R(1, 2) = state.transform[6];
    R(2, 0) = state.transform[8];
    R(2, 1) = state.transform[9];
    R(2, 2) = state.transform[10];

    Eigen::Quaterniond quat(R.transpose()); // require transpose otherwise rotation is inverted
    tf_stamped_.transform.rotation.x = quat.x();
    tf_stamped_.transform.rotation.y = quat.y();
    tf_stamped_.transform.rotation.z = quat.z();
    tf_stamped_.transform.rotation.w = quat.w();

    // Send transform
    tf_bc_->sendTransform(tf_stamped_);

    // Pack joint states
    js_.header.stamp = stamp;

    for (uint i = 0; i < 3; ++i) {
      q_(i) = state.joint_angles[i];
      q_(i + 3) = state.gimbal_angles[i];
      js_.effort[i] = state.joint_torque[i];
      js_.effort[i + 3] = state.gimbal_torque[i];
    }

    if (q_prev_set_) {
      dq_ = (q_ - q_prev_) * (1.0 / static_cast<double>(params_.update_rate));
    } else {
      dq_.setZero();
    }

    q_prev_ = q_;
    q_prev_set_ = true;

    for (uint i = 0; i < 6; ++i) {
      js_.position[i] = q_(i);
      js_.velocity[i] = dq_(i);
    }

    // Publish joint states
    js_pub_->publish(js_);

    // Pack twist stamped
    ts_.header.stamp = stamp;
    ts_.header.frame_id = params_.child_frame_id;
    ts_.twist.linear.x = MM2M * state.velocity[0];
    ts_.twist.linear.y = MM2M * state.velocity[1];
    ts_.twist.linear.z = MM2M * state.velocity[2];

    ts_.twist.angular.x = state.angular_velocity[0];
    ts_.twist.angular.y = state.angular_velocity[1];
    ts_.twist.angular.z = state.angular_velocity[2];

    // Publish twist stamped
    ts_pub_->publish(ts_);
  };
};
} // namespace geomagic_touch_x

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(geomagic_touch_x::DeviceDriver)
