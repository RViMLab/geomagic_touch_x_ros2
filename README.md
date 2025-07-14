# Geomagic Touch X ROS 2 Driver

# Introduction
Node for using the Geomagic Touch X connected via ethernet with the Robot Operating System 2 (ROS 2).
Currently tested and working with ROS 2 Foxy (`Ubuntu 20.04`).

# Install
> [!NOTE]
> Please install the device drivers, see e.g. [install-3ds-touch-drivers-2022.sh](https://github.com/jhu-cisst-external/3ds-touch-openhaptics/tree/main) and the SDK, see e.g. [install-3ds-openhaptics-3.4.sh](https://github.com/jhu-cisst-external/3ds-touch-openhaptics/tree/main).

1. Clone this driver into your workspace, install dependencies, and build:
```shell
git clone https://github.com/RViMLab/geomagic_touch_x_ros2.git src/geomagic_touch_x_ros2
rosdep install --from-paths src -r -i -y
colcon build --symlink-install
```

2. Plug in the device to your computer via Ethernet.
3. Setup a new connection:
  * Name: Haptic TouchX
  * IPv4 Method: "Link-Local Only"
  * IPv6 Method: "Link-Local Only"

## Pair device
Run this each time you use the device.

1. Plug in the device to your computer via Ethernet.
2. Ensure the correct wired connection is selected.
3. `ros2 launch geomagic_touch_x setup.launch.py`
4. Click "Rescan for Devices", this should ensure we can find the device.
5. Click "Pairing", and just after click the pairing button on the device.
6. The device should successfully pair.
7. Click "Apply" and then "Ok".

If this doesn't work, then see the official documentation provided with your device.

## Calibrate the device
Run this each time you use the device after the device has been paired (see previous section).

1. `ros2 launch geomagic_touch_x calibrate.launch.py`
2. Click the "Select" tab.
3. Then click the next button (right arrow) at the bottom of the dialog box.
4. Move the stylus in each axis (X, Y, Z) to properly calibrate the device.
5. Once each axis is calibrated, its icon will turn green.
6. Step through each other test, and then once each is complete, click the cross to exit the diagonostic setup.

If this doesn't work, then see the official documentation provided with your device.

# `geomagic_touch_x_node`

This is the main ROS 2 component you need to launch to set/get the state of the haptic device.

## Parameters
* `update_rate` (int): rate to run driver at in Hz
* `frame_id` (string): frame ID for published stamped twist
* `child_frame_id` (string): child frame ID for published stamped twist
* `device_name` (string): the name of the device, this can be set during pairing (see above)

## Subscribed Topics
* `~/command/wrench` (`geometry_msgs/Wrench`): the force that should be commanded at the device end-effector. Note, only the linear part is used for the Touch X device.

## Published Topics
* `~/twist` (`geometry_msgs/Twist`): the linear and angular velocity of the device end-effector.
* `~/joint_states` (`sensor_msgs/JointState`): the joint states of the device.

## Transform (`tf2`)
The transform of the device end-effector is broadcast using the `tf2` library.
The base frame is `touch_x_base` and the child frame is `touch_x_ee`.
