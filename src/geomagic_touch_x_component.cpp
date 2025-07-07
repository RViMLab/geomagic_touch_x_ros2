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
  DeviceDriver(const rclcpp::NodeOptions &options) : Node("geomagic_touch_x", options) {
    this->declare_params_();
    this->get_params_();
    this->init_pubs_(); // cheers ;)
    this->init_subs_();
    ctrl_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(static_cast<ino64_t>(1. / params_.update_rate)),
        std::bind(&DeviceDriver::on_update_, this));
    RCLCPP_INFO(this->get_logger(), "Instantiating device %s...", params_.device_name.c_str());
    device_ = std::make_unique<Device>(params_.device_name);
    RCLCPP_INFO(this->get_logger(), "Done.");
  };

protected:
  // device
  std::unique_ptr<Device> device_;

  // parameters
  DriverParams params_;

  // device state
  DeviceState state_;

  // publishers
  rclcpp::TimerBase::SharedPtr ctrl_timer_;
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr js_pub_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr ts_pub_;

  // subscribers
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
  void init_pubs_() {
    js_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("~/joint_states",
                                                                   rclcpp::SensorDataQoS());
    ts_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>("~/twist",
                                                                       rclcpp::SensorDataQoS());
  };
  void init_subs_() {
    w_sub_ = this->create_subscription<geometry_msgs::msg::Wrench>(
        "~/command/wrench", rclcpp::SensorDataQoS(),
        [this](geometry_msgs::msg::Wrench::SharedPtr msg) {
          // device_->get_state().cmd_force[0] = msg->force.x;
          // device_->get_state().cmd_force[1] = msg->force.y;
          // device_->get_state().cmd_force[2] = msg->force.z;
        });
  };
  void on_update_() {};
};
} // namespace geomagic_touch_x

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(geomagic_touch_x::DeviceDriver)

// int main(int argc, char **argv) {
//   // // Setup constants
//   // const double mm_to_m=0.001;
//   // const int hz = 200;
//   // const double dt = 1.0/static_cast<double>(hz);

//   // // Setup tf broadcaster
//   // static tf2_ros::TransformBroadcaster br;
//   // geometry_msgs::TransformStamped tf;
//   // tf.header.frame_id = "touch_x_base";
//   // tf.child_frame_id = "touch_x_ee";

//   // // Setup twist publisher
//   // ros::Publisher twist_pub = nh.advertise<geometry_msgs::TwistStamped>("twist", 1000);
//   // geometry_msgs::TwistStamped twist;

//   // // Setup joint state publisher
//   // ros::Publisher joint_state_pub = nh.advertise<sensor_msgs::JointState>("joint_states",
//   1000);
//   // bool q_prev_set = false;
//   // Eigen::VectorXd q(6), q_prev(6), dqdt(6);
//   // sensor_msgs::JointState joint_state;
//   // joint_state.name.push_back("joint_angle_1");
//   // joint_state.name.push_back("joint_angle_2");
//   // joint_state.name.push_back("joint_angle_3");
//   // joint_state.name.push_back("gimbal_angle_1");
//   // joint_state.name.push_back("gimbal_angle_2");
//   // joint_state.name.push_back("gimbal_angle_3");
//   // for (int i = 0; i < 6; ++i) {
//   //   joint_state.position.push_back(0.);
//   //   joint_state.velocity.push_back(0.);
//   //   joint_state.effort.push_back(0.);
//   // }

//   // // Get device name
//   // std::string device_name;
//   // nh.getParam("device_name", device_name);
//   // if (!(device_name.length() > 0)) {
//   //   ROS_ERROR("No device name given, shutting down geomagic_touch_x_node!");
//   //   return -1;
//   // }
//   // ROS_INFO("Haptic device name is \"%s\".", device_name.c_str());

//   // // Initialize haptic device
//   // HHD hHD = hdInitDevice(device_name.c_str());

//   // // Enable force output, i.e. all motors are turned on.
//   // if (!hdIsEnabled(HD_FORCE_OUTPUT)) {
//   //   hdEnable(HD_FORCE_OUTPUT);
//   //   ROS_INFO("Force output is enabled for the device \"%s\".", device_name.c_str());
//   // } else {
//   //   ROS_ERROR("Did not enable force output.");
//   //   return -1;
//   // }

//   // // Setup device state
//   // HapticDeviceState state;
//   // for (int i = 0; i < 3; ++i)
//   //   state.cmd_force[i] = 0.;

//   // // Start state publisher
//   // hdScheduleAsynchronous(device_state_callback, &state, HD_DEFAULT_SCHEDULER_PRIORITY);

//   // // Start the scheduler
//   // hdStartScheduler();

//   // // Start cmd_force listener
//   // CmdForceListener cmd_force_listener = CmdForceListener(&state, &nh);

//   // // Setup for main state publisher loop
//   // ros::Rate rate(hz); // runs at 200 Hz

//   // // Publish state
//   // while (ros::ok()) {

//   //   // Get time
//   //   ros::Time now = ros::Time::now();

//   //   // Pack transform stamp and translation
//   //   tf.header.stamp = now;
//   //   tf.transform.translation.x = mm_to_m * state.transform[12];
//   //   tf.transform.translation.y = mm_to_m * state.transform[13];
//   //   tf.transform.translation.z = mm_to_m * state.transform[14];

//   //   // Convert state transform to rotation matrix, then quaternion, and pack transform
//   //   Eigen::Matrix3d R(3, 3);
//   //   R(0, 0) = state.transform[0];
//   //   R(0, 1) = state.transform[1];
//   //   R(0, 2) = state.transform[2];
//   //   R(1, 0) = state.transform[4];
//   //   R(1, 1) = state.transform[5];
//   //   R(1, 2) = state.transform[6];
//   //   R(2, 0) = state.transform[8];
//   //   R(2, 1) = state.transform[9];
//   //   R(2, 2) = state.transform[10];

//   //   Eigen::Quaterniond quat(R.transpose()); // require transpose otherwise rotation is
//   inverted
//   //   tf.transform.rotation.x = quat.x();
//   //   tf.transform.rotation.y = quat.y();
//   //   tf.transform.rotation.z = quat.z();
//   //   tf.transform.rotation.w = quat.w();

//   //   // Send transform
//   //   br.sendTransform(tf);

//   //   // Pack joint states
//   //   joint_state.header.stamp = now;

//   //   for (int i = 0; i < 3; ++i) {
//   //     q(i) = state.joint_angles[i];
//   //     q(i+3) = state.gimbal_angles[i];
//   //     joint_state.effort[i] = state.joint_torque[i];
//   //     joint_state.effort[i+3] = state.gimbal_torque[i];
//   //   }

//   //   if (q_prev_set) {
//   //     dqdt = (q - q_prev)/dt;
//   //   } else {
//   //     dqdt = Eigen::VectorXd::Zero(6);
//   //   }

//   //   q_prev = q;
//   //   q_prev_set = true;

//   //   for (int i = 0; i < 6; ++i) {
//   //     joint_state.position[i] = q(i);
//   //     joint_state.velocity[i] = dqdt(i);
//   //   }

//   //   // Publish joint states
//   //   joint_state_pub.publish(joint_state);

//   //   // Pack twist
//   //   twist.header.stamp = now;
//   //   twist.twist.linear.x = mm_to_m * state.velocity[0];
//   //   twist.twist.linear.y = mm_to_m * state.velocity[1];
//   //   twist.twist.linear.z = mm_to_m * state.velocity[2];

//   //   twist.twist.angular.x = state.angular_velocity[0];
//   //   twist.twist.angular.y = state.angular_velocity[1];
//   //   twist.twist.angular.z = state.angular_velocity[2];

//   //   // Publish twist
//   //   twist_pub.publish(twist);

//   //   // Spin and sleep
//   //   ros::spinOnce();
//   //   rate.sleep();

//   // }

//   // // Cleanup
//   // hdStopScheduler();
//   // hdDisable(HD_FORCE_OUTPUT);
//   // hdDisableDevice(hHD);

//   return 0;
// }
