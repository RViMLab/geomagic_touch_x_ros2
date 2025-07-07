from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution


def generate_launch_description() -> LaunchDescription:
    ld = LaunchDescription()
    # launch arguments
    ld.add_action(DeclareLaunchArgument(name="device_name", default_value="left"))
    ld.add_action(
        DeclareLaunchArgument(
            name="update_rate",
            default_value="200",
        )
    )
    ld.add_action(DeclareLaunchArgument(name="frame_id", default_value="touch_x_base"))
    ld.add_action(
        DeclareLaunchArgument(name="child_frame_id", default_value="touch_x_ee")
    )
    ld.add_action(DeclareLaunchArgument(name="rviz", default_value="false"))
    # nodes
    ld.add_action(
        Node(
            package="geomagic_touch_x",
            executable="device_driver",
            name="geomagic_touch_x",
            output="screen",
            parameters=[
                {
                    "device_name": LaunchConfiguration("device_name"),
                    "update_rate": LaunchConfiguration("update_rate"),
                    "frame_id": LaunchConfiguration("frame_id"),
                    "child_frame_id": LaunchConfiguration("child_frame_id"),
                }
            ],
        )
    )
    ld.add_action(
        Node(
            package="rviz2",
            executable="rviz2",
            name="rviz2",
            output="screen",
            condition=IfCondition(LaunchConfiguration("rviz")),
            arguments=[
                "-d",
                PathJoinSubstitution(
                    [FindPackageShare("geomagic_touch_x"), "config/rviz.rviz"]
                ),
            ],
        )
    )
    return ld
