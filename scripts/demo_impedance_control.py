#!/usr/bin/python3
import numpy as np
import rclpy
import rclpy.time
import tf2_ros
from geometry_msgs.msg import TwistStamped, Wrench
from rclpy.node import Node

K = 45.0  # stiffness
D = 2.5  # velocity damping
G = 0.001  # acceleration damping


class ImpedanceNode(Node):

    hz = 100
    dt = 1.0 / float(hz)

    def __init__(self):
        super().__init__("demo_node")

        self.declare_parameters(
            "",
            [
                ("Kx", K),
                ("Ky", K),
                ("Kz", K),
                ("Dx", D),
                ("Dy", D),
                ("Dz", D),
                ("Gx", G),
                ("Gy", G),
                ("Gz", G),
                ("fig8", False),
            ],
        )

        Kx = self.get_parameter("Kx").get_parameter_value().double_value
        Ky = self.get_parameter("Ky").get_parameter_value().double_value
        Kz = self.get_parameter("Kz").get_parameter_value().double_value
        self.K = np.diag([Kx, Ky, Kz])

        Dx = self.get_parameter("Dx").get_parameter_value().double_value
        Dy = self.get_parameter("Dy").get_parameter_value().double_value
        Dz = self.get_parameter("Dz").get_parameter_value().double_value
        self.D = np.diag([Dx, Dy, Dz])

        Gx = self.get_parameter("Gx").get_parameter_value().double_value
        Gy = self.get_parameter("Gy").get_parameter_value().double_value
        Gz = self.get_parameter("Gz").get_parameter_value().double_value
        self.G = np.diag([Gx, Gy, Gz])

        self.tf_buffer = tf2_ros.Buffer()
        self.wrench_pub = self.create_publisher(Wrench, "command/wrench", 1)
        self.ee_vel = np.zeros(3)
        self.ee_acc = np.zeros(3)
        self.dt_vel = 1.0 / 200.0  # twist messages are published at 200Hz
        self.ts_sub = self.create_subscription(TwistStamped, "twist", self.on_twist, 1)
        self.tf_listener = tf2_ros.TransformListener(self.tf_buffer, self)
        self.ee_pos_goal = np.zeros(3)
        self.ee_vel_goal = np.zeros(3)
        self.ee_acc_goal = np.zeros(3)
        self.fig8 = self.get_parameter("fig8").get_parameter_value().bool_value
        self.start_time = self.get_time_in_seconds()
        self.timer = self.create_timer(self.dt, self.on_timer)

    def get_time_in_seconds(self):
        seconds, nanoseconds = self.get_clock().now().seconds_nanoseconds()
        return seconds + nanoseconds / 1.0e9

    def on_twist(self, msg: TwistStamped):
        ee_vel = np.array([msg.twist.linear.x, msg.twist.linear.y, msg.twist.linear.z])
        self.ee_acc = (ee_vel - self.ee_vel) / self.dt_vel
        self.ee_vel = ee_vel

    def get_ee_pos(self):
        try:
            tf = self.tf_buffer.lookup_transform(
                "touch_x_base", "touch_x_ee", rclpy.time.Time()
            )
        except:
            tf = None
        return tf

    def on_timer(self):

        # Get ee position
        ee = self.get_ee_pos()
        if ee is None:
            return
        ee_pos = np.array(
            [
                ee.transform.translation.x,
                ee.transform.translation.y,
                ee.transform.translation.z,
            ]
        )

        # Update goal position
        if self.fig8:
            t = self.get_time_in_seconds() - self.start_time
            self.ee_pos_goal[0] = 0.04 * np.sin(t * np.pi * 0.5)
            self.ee_pos_goal[1] = 0.04 * np.sin(t * np.pi)

        # Impedance controller
        f = (
            self.K @ (self.ee_pos_goal - ee_pos)
            + self.D @ (self.ee_vel_goal - self.ee_vel)
            + self.G @ (self.ee_acc_goal - self.ee_acc)
        )

        # Publish wrench
        msg = Wrench()
        msg.force.x = f[0]
        msg.force.y = f[1]
        msg.force.z = f[2]
        self.wrench_pub.publish(msg)


def main():
    rclpy.init()
    impedanace_node = ImpedanceNode()
    try:
        rclpy.spin(impedanace_node)
    except KeyboardInterrupt:
        pass
    rclpy.shutdown()


if __name__ == "__main__":
    main()
