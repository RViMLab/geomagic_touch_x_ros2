from launch_ros.actions import Node

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration


def generate_launch_description() -> LaunchDescription:
    ld = LaunchDescription()
    # launch arguments
    ld.add_action(DeclareLaunchArgument(name="fig8", default_value="false"))
    ld.add_action(
        DeclareLaunchArgument(
            name="namespace",
            default_value="geomagic_touch_x",
            description="Namespace for the demo node. Must match the device driver node name.",
        )
    )
    # nodes
    ld.add_action(
        Node(
            package="geomagic_touch_x",
            name="demo_node",
            executable="demo_impedance_control.py",
            output="screen",
            namespace=LaunchConfiguration("namespace"),
            remappings=[("tf", "/tf"), ("tf_static", "/tf_static")],
            parameters=[{"fig8": LaunchConfiguration("fig8")}],
        )
    )
    return ld
