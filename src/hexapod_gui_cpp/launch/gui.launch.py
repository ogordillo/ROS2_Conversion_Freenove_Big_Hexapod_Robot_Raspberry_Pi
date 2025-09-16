from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    """
    Launches the C++ Hexapod GUI node.
    """
    
    # Use the 'Node' action to directly launch the C++ executable
    start_gui_node = Node(
        package='hexapod_gui_cpp',       # The name of your new C++ package
        executable='hexapod_gui_main',   # The name of the executable from your CMakeLists.txt
        name='hexapod_gui',              # A friendly name for the node as it runs
        output='screen',                 # Show node output in the terminal
        emulate_tty=True                 # Ensure terminal colors and formatting are correct
    )

    return LaunchDescription([
        start_gui_node
    ])