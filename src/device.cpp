#include "geomagic_touch_x/device.hpp"

namespace geomagic_touch_x {
void Device::open(const std::string &device_name) {
  if (is_open_)
    return;
  if (device_name.empty()) {
    throw std::runtime_error("No emtpy device_name allowed.");
  }
  // Initialize haptic device
  hHD_ptr_ = std::make_unique<HHD>(hdInitDevice(device_name.c_str()));
  // Enable force output, i.e. all motors are turned on.
  if (!hdIsEnabled(HD_FORCE_OUTPUT)) {
    hdEnable(HD_FORCE_OUTPUT);
  } else {
    throw std::runtime_error("Failed to enable force output.");
  }
  // Setup device state
  for (int i = 0; i < 3; ++i)
    state_.cmd_force[i] = 0.;
  // Start state publisher
  hdScheduleAsynchronous(this->on_device_state_, &state_, HD_DEFAULT_SCHEDULER_PRIORITY);
  // Start the scheduler
  hdStartScheduler();
  is_open_ = true;
}

void Device::close() {
  if (!is_open_)
    return;
  hdStopScheduler();
  hdDisable(HD_FORCE_OUTPUT);
  hdDisableDevice(*hHD_ptr_);
  hHD_ptr_.reset();
  is_open_ = false;
}

HDCallbackCode HDCALLBACK Device::on_device_state_(void *data) {
  // Setup
  State *state = (State *)data;

  // Begin frame
  hdBeginFrame(hdGetCurrentDevice());

  // Get device state
  hdGetDoublev(HD_CURRENT_POSITION, state->position);
  hdGetDoublev(HD_CURRENT_VELOCITY, state->velocity);
  hdGetDoublev(HD_CURRENT_TRANSFORM, state->transform);
  hdGetDoublev(HD_CURRENT_ANGULAR_VELOCITY, state->angular_velocity);
  hdGetDoublev(HD_CURRENT_JOINT_ANGLES, state->joint_angles);
  hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, state->gimbal_angles);
  hdGetDoublev(HD_CURRENT_FORCE, state->force);
  hdGetDoublev(HD_CURRENT_TORQUE, state->torque);
  hdGetDoublev(HD_CURRENT_JOINT_TORQUE, state->joint_torque);
  hdGetDoublev(HD_CURRENT_GIMBAL_TORQUE, state->gimbal_torque);

  // Set device force
  hdSetDoublev(HD_CURRENT_FORCE, state->cmd_force);

  // End frame
  hdEndFrame(hdGetCurrentDevice());

  // Check for error
  HDErrorInfo error;
  if (HD_DEVICE_ERROR(error = hdGetError())) {
    hduPrintError(stderr, &error, "Error during main scheduler callback\n");
    if (hduIsSchedulerError(&error))
      return HD_CALLBACK_DONE;
  }

  return HD_CALLBACK_CONTINUE;
}
} // namespace geomagic_touch_x