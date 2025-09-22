#!/usr/bin/env python3

"""
Camera control and display popup window for the Hexapod GUI.
"""

import numpy as np
import cv2
from functools import partial
from PyQt5.QtWidgets import (QWidget, QLabel, QPushButton, QFrame, QVBoxLayout,
                             QHBoxLayout, QGridLayout, QSlider, QLineEdit)
from PyQt5.QtCore import Qt, pyqtSlot
from PyQt5.QtGui import QImage, QPixmap, QDoubleValidator

from ..config import SERVO_CHANNELS, RVIZ_JOINT_NAMES

class CameraDisplayWindow(QWidget):
    """A popup window for camera video streams and pan/tilt control."""

    def __init__(self, ros_node_thread, shared_joint_states, parent=None):
        super().__init__(parent)
        self.ros_node = ros_node_thread
        self._joint_states = shared_joint_states

        self.setWindowTitle("Camera Control & Display")
        self.setMinimumSize(1300, 600)
        self.main_layout = QVBoxLayout(self)

        self._create_control_frame()
        self._create_video_frame()

        self.ros_node.color_image_signal.connect(self._update_color_image)
        self.ros_node.depth_image_signal.connect(self._update_depth_image)

    def _create_control_frame(self):
        frame = QFrame()
        frame.setFrameShape(QFrame.StyledPanel)
        layout = QGridLayout(frame)
        layout.addWidget(QLabel("<b>Camera Servo Control</b>"), 0, 0, 1, 3)
        
        pan_handler = partial(self._send_camera_command, 'pan', SERVO_CHANNELS['camera']['pan'])
        pan_controls = self._create_control_triplet("Pan", 0, 180, pan_handler)
        layout.addWidget(pan_controls['label'], 1, 0)
        layout.addWidget(pan_controls['slider'], 1, 1)
        layout.addWidget(pan_controls['textbox'], 1, 2)
        
        tilt_handler = partial(self._send_camera_command, 'tilt', SERVO_CHANNELS['camera']['tilt'])
        tilt_controls = self._create_control_triplet("Tilt", 0, 180, tilt_handler)
        layout.addWidget(tilt_controls['label'], 2, 0)
        layout.addWidget(tilt_controls['slider'], 2, 1)
        layout.addWidget(tilt_controls['textbox'], 2, 2)

        self.main_layout.addWidget(frame)

    def _create_video_frame(self):
        frame = QFrame()
        frame.setFrameShape(QFrame.StyledPanel)
        layout = QHBoxLayout(frame)

        # Color Stream
        color_vbox = QVBoxLayout()
        self.color_view = self._create_video_label("Color Stream Off")
        self.color_toggle_btn = self._create_toggle_button("Enable Color Stream", 'color')
        color_vbox.addWidget(self.color_view)
        color_vbox.addWidget(self.color_toggle_btn)

        depth_vbox = QVBoxLayout()
        self.depth_view = self._create_video_label("Depth Stream Off")
        self.depth_toggle_btn = self._create_toggle_button("Enable Depth Stream", 'depth')
        depth_vbox.addWidget(self.depth_view)
        depth_vbox.addWidget(self.depth_toggle_btn)
        
        layout.addLayout(color_vbox)
        layout.addLayout(depth_vbox)
        self.main_layout.addWidget(frame)

    def _create_video_label(self, text):
        label = QLabel(text)
        label.setFixedSize(640, 480)
        label.setStyleSheet("background-color: black; color: white;")
        label.setAlignment(Qt.AlignCenter)
        return label
    
    def _create_toggle_button(self, text, stream_type):
        button = QPushButton(text)
        button.setCheckable(True)
        button.toggled.connect(partial(self._toggle_video_stream, stream_type))
        return button

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
    
    def _send_camera_command(self, servo_name, channel, angle):
        print(f"COMMAND: Camera={servo_name}, Channel={channel}, Angle={angle}")
        if not self.ros_node.sim_mode:
            self.ros_node.call_set_servo(channel, angle)
        
        angle_rad = np.deg2rad(angle - 90)
        rviz_joint_name = RVIZ_JOINT_NAMES['camera'].get(servo_name)
        if rviz_joint_name:
            self._joint_states[rviz_joint_name] = angle_rad
            self.ros_node.publish_all_joint_states(self._joint_states)

    def _toggle_video_stream(self, stream_type, checked):
        btn = self.color_toggle_btn if stream_type == 'color' else self.depth_toggle_btn
        if checked:
            self.ros_node.subscribe_to_video(stream_type)
            btn.setText(f"Disable {stream_type.capitalize()} Stream")
            btn.setStyleSheet("background-color: #ff6b6b;")
        else:
            self.ros_node.unsubscribe_from_video(stream_type)
            btn.setText(f"Enable {stream_type.capitalize()} Stream")
            btn.setStyleSheet("")
            view = self.color_view if stream_type == 'color' else self.depth_view
            view.setText(f"{stream_type.capitalize()} Stream Off")
            view.setStyleSheet("background-color: black; color: white;")

    @pyqtSlot(np.ndarray)
    def _update_color_image(self, cv_img):
        qt_img = self._convert_cv_qt(cv_img)
        self.color_view.setPixmap(qt_img)

    @pyqtSlot(np.ndarray)
    def _update_depth_image(self, cv_img):
        cv_img_normalized = cv2.normalize(cv_img, None, 0, 255, cv2.NORM_MINMAX, dtype=cv2.CV_8U)
        cv_img_colormap = cv2.applyColorMap(cv_img_normalized, cv2.COLORMAP_JET)
        qt_img = self._convert_cv_qt(cv_img_colormap)
        self.depth_view.setPixmap(qt_img)

    def _convert_cv_qt(self, cv_img, width=640, height=480):
        if len(cv_img.shape) == 3:
            h, w, ch = cv_img.shape
            bytes_per_line = ch * w
            qt_format = QImage(cv_img.data, w, h, bytes_per_line, QImage.Format_BGR888)
        else:
            h, w = cv_img.shape
            bytes_per_line = w
            qt_format = QImage(cv_img.data, w, h, bytes_per_line, QImage.Format_Grayscale8)
        
        pixmap = QPixmap.fromImage(qt_format)
        return pixmap.scaled(width, height, Qt.KeepAspectRatio)
    
    def closeEvent(self, event):
        if self.color_toggle_btn.isChecked(): self.color_toggle_btn.toggle()
        if self.depth_toggle_btn.isChecked(): self.depth_toggle_btn.toggle()
        super().closeEvent(event)