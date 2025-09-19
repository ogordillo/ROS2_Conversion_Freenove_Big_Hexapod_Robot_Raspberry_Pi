#!/usr/bin/env python3

"""
Configuration file for the Hexapod Robot Control GUI.

This module centralizes all static configuration data, including ROS topics,
service names, servo channel mappings, and joint name definitions for RViz
and Gazebo.
"""

import math

# --- Path for Zenoh configuration ---
ZENOH_CONFIG_PATH = '/root/config/client.yaml'

# --- Servo Channel Definitions ---
# Maps component joints to their corresponding servo channel IDs.
SERVO_CHANNELS = {
    'camera': {'pan': 0, 'tilt': 1},
    'leg1': {'coxa': 15, 'femur': 14, 'tibia': 13},
    'leg2': {'coxa': 12, 'femur': 11, 'tibia': 10},
    'leg3': {'coxa': 9, 'femur': 8, 'tibia': 31},
    'leg4': {'coxa': 22, 'femur': 23, 'tibia': 27},
    'leg5': {'coxa': 19, 'femur': 20, 'tibia': 21},
    'leg6': {'coxa': 16, 'femur': 17, 'tibia': 18},
}

# --- Reverse Servo Map ---
# Creates a reverse mapping from channel ID back to component and joint name.
REVERSE_SERVO_MAP = {
    channel: {'component': component, 'joint': joint}
    for component, joints in SERVO_CHANNELS.items()
    for joint, channel in joints.items()
}

# --- RViz/Gazebo Joint Names ---
# Maps component joints to their corresponding names in the robot's URDF.
RVIZ_JOINT_NAMES = {
    'leg1': {'coxa': 'leg1_coxa_joint', 'femur': 'leg1_femur_joint', 'tibia': 'leg1_tibia_joint'},
    'leg2': {'coxa': 'leg2_coxa_joint', 'femur': 'leg2_femur_joint', 'tibia': 'leg2_tibia_joint'},
    'leg3': {'coxa': 'leg3_coxa_joint', 'femur': 'leg3_femur_joint', 'tibia': 'leg3_tibia_joint'},
    'leg4': {'coxa': 'leg4_coxa_joint', 'femur': 'leg4_femur_joint', 'tibia': 'leg4_tibia_joint'},
    'leg5': {'coxa': 'leg5_coxa_joint', 'femur': 'leg5_femur_joint', 'tibia': 'leg5_tibia_joint'},
    'leg6': {'coxa': 'leg6_coxa_joint', 'femur': 'leg6_femur_joint', 'tibia': 'leg6_tibia_joint'},
    'camera': {'pan': 'camera_pan_joint', 'tilt': 'camera_tilt_joint'},
}

# --- Helper Lists and Dictionaries ---
# A flattened list of all URDF joint names for easier iteration.
ALL_RVIZ_JOINT_NAMES = [
    rviz_name
    for component in RVIZ_JOINT_NAMES.keys()
    for rviz_name in RVIZ_JOINT_NAMES[component].values()
]

# Initial state for all joints, set to 0.0 radians.
INITIAL_JOINT_STATES = {joint_name: 0.0 for joint_name in ALL_RVIZ_JOINT_NAMES}

# Maps URDF joint names directly to servo channel IDs.
JOINT_NAME_TO_CHANNEL_MAP = {
    joint_name: SERVO_CHANNELS[component][joint_type]
    for component, joints in RVIZ_JOINT_NAMES.items()
    for joint_type, joint_name in joints.items()
}

# --- ROS2 Topics and Services ---
ROS_TOPICS = {
    'color': '/camera/camera/color/image_raw',
    'depth': '/camera/camera/depth/image_rect_raw',
    'gazebo_cmd': '/hexapod_joint_group_controller/command',
    'imu_real': '/imu/data_raw',
    'imu_sim': '/imu/data_sim',
    'joint_states': '/joint_states',
    'battery1': '/battery1_state',
    'battery2': '/battery2_state'
}

ROS_SERVICES = {
    'set_servo': '/set_servo_angle'
}