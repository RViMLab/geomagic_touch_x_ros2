from launch import LaunchDescription
from launch.actions import ExecuteProcess


def generate_launch_description() -> LaunchDescription:
    ld = LaunchDescription()
    ld.add_action(
        ExecuteProcess(
            cmd=["/opt/geomagic_touch_device_driver/Geomagic_Touch_Setup"]
        )
    )
    return ld
