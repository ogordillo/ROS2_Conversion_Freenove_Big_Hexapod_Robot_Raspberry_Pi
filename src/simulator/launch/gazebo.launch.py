import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument, RegisterEventHandler, AppendEnvironmentVariable
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from launch.substitutions import LaunchConfiguration
import xacro

def generate_launch_description():

    pkg_ros_gz_sim = get_package_share_directory('ros_gz_sim')
    pkg_hexapod_description = get_package_share_directory('simulator')
    
    ros_gz_bridge_config = os.path.join(pkg_hexapod_description, 'config', 'ros_gz_bridge_gazebo.yaml')
    gui_config_path = os.path.join(pkg_hexapod_description, "config", "gui.config")
    robot_description_file = os.path.join(pkg_hexapod_description, 'urdf', 'hexapod_model.xacro')
    robot_description_config = xacro.process_file(robot_description_file)
    world_file = os.path.join(pkg_hexapod_description, 'worlds', 'hexapod_world.sdf')
    
    
    
    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_ros_gz_sim, 'launch', 'gz_sim.launch.py')
        ),
        launch_arguments={'gz_args': [f'-r -v 4 --gui-config ', gui_config_path]}.items(),
    )

    use_sim_time = LaunchConfiguration('use_sim_time', default='true')

    node_robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time, 'robot_description': robot_description_config.toxml()}],
    )

    spawn_entity = Node(
        package='ros_gz_sim', 
        executable='create',
        arguments=['-topic', 'robot_description',
                   '-name', 'servo_robot',
                   '-z', '0.5'],
        output='screen'
    )

    imu_filter_node = Node(
        package='simulator',
        executable='imu_filter_node',
        name='gazebo_imu_filter_node',
        output='screen'
    )

    bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=[
            "/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock",
            "/imu/data@sensor_msgs/msg/Imu[gz.msgs.IMU",
            "/imu/data_sim@sensor_msgs/msg/Imu[gz.msgs.IMU"
        ],
        # remappings=[('/imu/data', '/imu/data_sim_raw')],
        output='screen',
        parameters=[
                {'use_sim_time': True}
            ]
    )

    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "--controller-manager", "/controller_manager"],
    )

    joint_trajectory_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_trajectory_controller", "--controller-manager", "/controller_manager"],
    )

    delay_joint_trajectory_controller_spawner = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=spawn_entity,
            on_exit=[joint_state_broadcaster_spawner,
                     joint_trajectory_controller_spawner],
        )
    )


    return LaunchDescription([
        DeclareLaunchArgument(
          'use_sim_time',
          default_value='false',
          description='Use simulation (Gazebo) clock if true'),
        gz_sim,
        node_robot_state_publisher,
        bridge,
        spawn_entity,
        delay_joint_trajectory_controller_spawner,
        imu_filter_node
    ])

    