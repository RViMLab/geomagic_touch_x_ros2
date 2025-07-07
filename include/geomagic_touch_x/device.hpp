#ifndef GEOMAGIC_TOUCH_X__DEVICE_HPP_
#define GEOMAGIC_TOUCH_X__DEVICE_HPP_

#include <memory>
#include <stdexcept>

#include <HD/hdDefines.h>
#include <HD/hdDevice.h>
#include <HD/hdScheduler.h>
#include <HDU/hduError.h>

namespace geomagic_touch_x {
class Device {
public:
  struct State {
    // current position of the device facing the device base. Right is
    // +x, up is +y, toward user is +z.
    HDdouble position[3];

    // current velocity of the device.
    HDdouble velocity[3];

    // column-major transform of the device end-effector.
    HDdouble transform[16];

    // angular velocity of the device gimbal.
    HDdouble angular_velocity[3];

    // joint angles of the device. These are joint angles used for
    // computing the kinematics of the armature relative to the base
    // frame of the device. For Touch devices: Turet Left +, Thigh Up +,
    // Shin Up +
    HDdouble joint_angles[3];

    // angles of the device gimbal. For Touch devices: From Neutral
    // position Right is +, Up is -, CW is +
    HDdouble gimbal_angles[3];

    // current force, i.e. the force that the user is commanding to the
    // device during the frame in which this is called.
    HDdouble force[3];

    // current torque, i.e. the torque that the user is commanding to
    // the device during the frame in which this is called.
    HDdouble torque[3];

    // current joint torque, i.e. the torque the user is commanding to
    // the first 3 joints of the device during the frame in which this
    // is called
    HDdouble joint_torque[3];

    // current gimbal torque, i.e. the gimbal torque the user is
    // commanding to the device during the frame in which this is
    // called.
    HDdouble gimbal_torque[3];

    // Force to command at end-effector
    HDdouble cmd_force[3];
  };

public:
  Device(const std::string &device_name) : is_open_(false) { this->open(device_name); }
  ~Device() { this->close(); }

  inline State &get_state() { return state_; }; // note this isn't (wasn't) thread safe

protected:
  std::unique_ptr<HHD> hHD_ptr_;
  bool is_open_;
  State state_;

public:
  void open(const std::string &device_name);
  void close();

protected:
  static HDCallbackCode HDCALLBACK on_device_state_(void *data);
};
} // namespace geomagic_touch_x
#endif // GEOMAGIC_TOUCH_X__DEVICE_HPP_