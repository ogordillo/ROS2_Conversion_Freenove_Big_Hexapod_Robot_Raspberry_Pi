#!/usr/bin/env python3

"""
ROS2 Communication Handler for the Hexapod GUI.

This module contains the RosNodeThread class, which manages all ROS2
communications (publishers, subscribers, service clients) in a separate,
non-blocking background thread to keep the GUI responsive.
"""

import rclpy
import cv2
import numpy as np
import math
from rclpy.node import Node
from PyQt5.QtCore import QThread, pyqtSignal

from cv_bridge import CvBridge
from sensor_msgs.msg import Image, JointState, BatteryState, Imu
from trajectory_msgs.msg import JointTrajectory, JointTrajectoryPoint
from builtin_interfaces.msg import Duration
from robot_interfaces.srv import SetServo

# Import configuration from the local package
from .config import (ROS_TOPICS, ROS_SERVICES, JOINT_NAME_TO_CHANNEL_MAP,
                     ALL_RVIZ_JOINT_NAMES)

class RosNodeThread(QThread):
    """ Manages all ROS2 communications in a background thread """
    color_image_signal = pyqtSignal(np.ndarray)
    depth_image_signal = pyqtSignal(np.ndarray)
    telemetry_update_signal = pyqtSignal(dict)

    def __init__(self):
        super().__init__()
        self.node = None
        self.bridge = CvBridge()
        self.color_sub = None
        self.depth_sub = None
        self.sim_mode = False
        self.imu_sub = None

    def run(self):
        """Initializes the ROS2 node and spins it."""
        try:
            rclpy.init()
        except Exception as e:
            print(f"Error during rclpy.init(): {e}")
            self.telemetry_update_signal.emit({'connection': False})
            return

        self.node = rclpy.create_node('hexapod_gui_node')
        self.set_servo_client = self.node.create_client(SetServo, ROS_SERVICES['set_servo'])
        self.joint_state_pub = self.node.create_publisher(JointState, ROS_TOPICS['joint_states'], 10)
        self.gazebo_joint_publisher = self.node.create_publisher(JointTrajectory, '/joint_trajectory_controller/joint_trajectory', 10)

        self.joint_state_sub = self.node.create_subscription(JointState, ROS_TOPICS['joint_states'], self._joint_state_callback, 10)
        self.battery1_sub = self.node.create_subscription(BatteryState, ROS_TOPICS['battery1'], self._battery1_callback, 10)
        self.battery2_sub = self.node.create_subscription(BatteryState, ROS_TOPICS['battery2'], self._battery2_callback, 10)

        self.update_subscriptions()

        self.node.get_logger().info("Hexapod GUI ROS2 Node is running.")
        rclpy.spin(self.node)

        self.node.destroy_node()

    def update_subscriptions(self):
        """Creates or recreates subscriptions based on the current mode (real/sim)."""
        if self.imu_sub is not None:
            self.node.destroy_subscription(self.imu_sub)
            self.imu_sub = None
            self.node.get_logger().info("Destroyed existing IMU subscription.")

        topic = ROS_TOPICS['imu_sim'] if self.sim_mode else ROS_TOPICS['imu_real']
        log_mode = "SIM" if self.sim_mode else "REAL"
        self.node.get_logger().info(f"{log_mode} MODE: Subscribing to IMU topic '{topic}'")

        self.imu_sub = self.node.create_subscription(
            Imu, topic, self._imu_callback, 10)

    def _imu_callback(self, msg):
        """Processes incoming IMU messages and updates orientation data."""
        # Quaternion to Euler angle conversion
        q = msg.orientation
        t0 = +2.0 * (q.w * q.x + q.y * q.z)
        t1 = +1.0 - 2.0 * (q.x * q.x + q.y * q.y)
        roll_x = math.atan2(t0, t1)

        t2 = +2.0 * (q.w * q.y - q.z * q.x)
        t2 = +1.0 if t2 > +1.0 else -1.0 if t2 < -1.0 else t2
        pitch_y = math.asin(t2)

        t3 = +2.0 * (q.w * q.z + q.x * q.y)
        t4 = +1.0 - 2.0 * (q.y * q.y + q.z * q.z)
        yaw_z = math.atan2(t3, t4)

        imu_data = {
            'x': msg.linear_acceleration.x, 'y': msg.linear_acceleration.y, 'z': msg.linear_acceleration.z,
            'roll': math.degrees(roll_x), 'pitch': math.degrees(pitch_y), 'yaw': math.degrees(yaw_z),
            'connection': True
        }
        self.telemetry_update_signal.emit(imu_data)

    def _battery1_callback(self, msg):
        """Processes incoming BatteryState messages for battery 1."""
        battery1_level = int(msg.percentage * 100) if msg.percentage >= 0 else 0
        self.telemetry_update_signal.emit({'battery1': battery1_level, 'connection': True})

    def _battery2_callback(self, msg):
        """Processes incoming BatteryState messages for battery 2."""
        battery2_level = int(msg.percentage * 100) if msg.percentage >= 0 else 0
        self.telemetry_update_signal.emit({'battery2': battery2_level, 'connection': True})

    def _joint_state_callback(self, msg):
        """Processes incoming JointState messages."""
        telemetry_data = {}
        for joint_name, position_rad in zip(msg.name, msg.position):
            if joint_name in JOINT_NAME_TO_CHANNEL_MAP:
                channel = JOINT_NAME_TO_CHANNEL_MAP[joint_name]
                angle_deg = math.degrees(position_rad) + 90.0
                telemetry_data[channel] = angle_deg
        if telemetry_data:
            self.telemetry_update_signal.emit(telemetry_data)

    def _color_callback(self, msg):
        cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')
        self.color_image_signal.emit(cv_image)

    def _depth_callback(self, msg):
        cv_image = self.bridge.imgmsg_to_cv2(msg, desired_encoding='passthrough')
        self.depth_image_signal.emit(cv_image)

    def subscribe_to_video(self, stream_type):
        """Subscribes to the specified video topic."""
        if stream_type == 'color' and self.color_sub is None:
            self.color_sub = self.node.create_subscription(Image, ROS_TOPICS['color'], self._color_callback, 10)
            self.node.get_logger().info(f"Subscribed to {ROS_TOPICS['color']}")
        elif stream_type == 'depth' and self.depth_sub is None:
            self.depth_sub = self.node.create_subscription(Image, ROS_TOPICS['depth'], self._depth_callback, 10)
            self.node.get_logger().info(f"Subscribed to {ROS_TOPICS['depth']}")

    def unsubscribe_from_video(self, stream_type):
        """Unsubscribes from the specified video topic."""
        if stream_type == 'color' and self.color_sub:
            self.node.destroy_subscription(self.color_sub)
            self.color_sub = None
            self.node.get_logger().info(f"Unsubscribed from {ROS_TOPICS['color']}")
        elif stream_type == 'depth' and self.depth_sub:
            self.node.destroy_subscription(self.depth_sub)
            self.depth_sub = None
            self.node.get_logger().info(f"Unsubscribed from {ROS_TOPICS['depth']}")

    def call_set_servo(self, channel, angle):
        """Calls the set_servo_angle service."""
        if not self.set_servo_client.wait_for_service(timeout_sec=1.0):
            self.node.get_logger().error(f"Service '{ROS_SERVICES['set_servo']}' not available.")
            self.telemetry_update_signal.emit({'connection': False})
            return
        req = SetServo.Request()
        req.channel = int(channel)
        req.angle = int(angle)
        self.set_servo_client.call_async(req)
        self.node.get_logger().info(f"Set servo channel {channel} to {angle} degrees.")

    def set_sim_mode(self, enabled):
        """Sets the simulation mode flag and updates subscriptions."""
        self.sim_mode = enabled
        if self.node:
            log_msg = "enabled" if enabled else "disabled"
            self.node.get_logger().info(f"Gazebo simulation publishing is {log_msg}.")
            self.update_subscriptions()

    def publish_all_joint_states(self, joint_states_dict):
        """Publishes joint states to RViz or Gazebo based on the current mode."""
        if not self.node: return

        if self.sim_mode:
            ordered_positions = [joint_states_dict.get(name, 0.0) for name in ALL_RVIZ_JOINT_NAMES]
            traj_msg = JointTrajectory()
            traj_msg.joint_names = ALL_RVIZ_JOINT_NAMES
            point = JointTrajectoryPoint()
            point.positions = ordered_positions
            point.time_from_start = Duration(sec=1, nanosec=0)
            traj_msg.points.append(point)
            self.node.get_logger().info(f'Sending command to move {len(traj_msg.joint_names)} joints.')
            self.gazebo_joint_publisher.publish(traj_msg)
        else:
            rviz_msg = JointState()
            rviz_msg.header.stamp = self.node.get_clock().now().to_msg()
            rviz_msg.name = list(joint_states_dict.keys())
            rviz_msg.position = list(joint_states_dict.values())
            self.joint_state_pub.publish(rviz_msg)

    def stop(self):
        """Shuts down the ROS2 node and waits for the thread to finish."""
        if rclpy.ok():
            rclpy.shutdown()
        self.wait()