from launch import LaunchDescription
from launch.actions import ExecuteProcess


def generate_launch_description() -> LaunchDescription:
    ld = LaunchDescription()
    ld.add_action(ExecuteProcess(cmd=["Touch_Setup"]))
    return ld
