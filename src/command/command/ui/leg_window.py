#!/usr/bin/env python3

"""
Leg control popup window for the Hexapod GUI.
"""

import numpy as np
from functools import partial
from PyQt5.QtWidgets import (QWidget, QLabel, QFrame, QVBoxLayout, QGridLayout,
                             QSlider, QLineEdit)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QDoubleValidator

from ..config import SERVO_CHANNELS, RVIZ_JOINT_NAMES

class LegDisplayWindow(QWidget):
    """A popup window for detailed control of a single leg."""

    def __init__(self, leg_name, ros_node_thread, shared_joint_states, parent=None):
        super().__init__(parent)
        self.leg_name = leg_name
        self.ros_node = ros_node_thread
        self.channels = SERVO_CHANNELS[leg_name.lower().replace(" ", "")]
        self.rviz_joints = RVIZ_JOINT_NAMES[leg_name.lower().replace(" ", "")]

        self._joint_states = shared_joint_states

        self.setWindowTitle(f"{leg_name} Control")
        self.setMinimumWidth(400)
        self.main_layout = QVBoxLayout(self)

        self._create_joint_control_frame()
        self._create_ik_control_frame()

    def _create_joint_control_frame(self):
        joint_frame = QFrame()
        joint_frame.setFrameShape(QFrame.StyledPanel)
        joint_layout = QGridLayout(joint_frame)
        joint_layout.addWidget(QLabel(f"<b>Joint Control ({self.leg_name})</b>"), 0, 0, 1, 3)
        
        self.controls = {}
        joints = ['coxa', 'femur', 'tibia']
        for i, joint in enumerate(joints):
            self.controls[joint] = self._create_control_triplet(
                joint.capitalize(), 0, 180,
                partial(self._send_joint_command, joint, self.channels[joint])
            )
            joint_layout.addWidget(self.controls[joint]['label'], i + 1, 0)
            joint_layout.addWidget(self.controls[joint]['slider'], i + 1, 1)
            joint_layout.addWidget(self.controls[joint]['textbox'], i + 1, 2)
        
        self.main_layout.addWidget(joint_frame)

    def _create_ik_control_frame(self):
        ik_frame = QFrame()
        ik_frame.setFrameShape(QFrame.StyledPanel)
        ik_layout = QGridLayout(ik_frame)
        ik_layout.addWidget(QLabel("<b>Inverse Kinematics (IK) Control</b>"), 0, 0, 1, 3)
        
        ik_params = ['X', 'Y', 'Z', 'Pitch', 'Yaw', 'Roll']
        for i, param in enumerate(ik_params):
            handler = lambda val, p=param: print(f"IK {p} set to {val}")
            ik_control = self._create_control_triplet(param, 0, 100, handler)
            ik_layout.addWidget(ik_control['label'], i + 1, 0)
            ik_layout.addWidget(ik_control['slider'], i + 1, 1)
            ik_layout.addWidget(ik_control['textbox'], i + 1, 2)

        self.main_layout.addWidget(ik_frame)

    def _create_control_triplet(self, name, min_val, max_val, handler_func):
        label = QLabel(name)
        slider = QSlider(Qt.Horizontal)
        slider.setRange(min_val, max_val)
        slider.setValue((min_val + max_val) // 2)
        textbox = QLineEdit(str(float(slider.value())))
        textbox.setValidator(QDoubleValidator(min_val, max_val, 2))
        textbox.setFixedWidth(50)
        
        slider.valueChanged.connect(lambda val, tb=textbox: tb.setText(f"{val:.1f}"))
        textbox.textChanged.connect(lambda txt, sl=slider: sl.setValue(int(float(txt)) if txt else 0))
        slider.sliderReleased.connect(lambda sl=slider: handler_func(sl.value()))
        textbox.editingFinished.connect(lambda tb=textbox, sl=slider: handler_func(sl.value()))
        
        return {'label': label, 'slider': slider, 'textbox': textbox}
    
    def _send_joint_command(self, joint_name, channel, angle):
        print(f"COMMAND: Leg={self.leg_name}, Joint={joint_name}, Channel={channel}, Angle={angle}")
        if not self.ros_node.sim_mode:
            self.ros_node.call_set_servo(channel, angle)
        
        angle_rad = np.deg2rad(angle - 90)
        rviz_joint_name = self.rviz_joints.get(joint_name)
        if rviz_joint_name:
            self._joint_states[rviz_joint_name] = angle_rad
        
        self.ros_node.publish_all_joint_states(self._joint_states)