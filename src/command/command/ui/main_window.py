#!/usr/_env python3

"""
Main window for the Hexapod Control GUI.

This module defines the main application window, which serves as the central
hub for telemetry display and launching control windows.
"""

import os
import yaml
from PyQt5.QtWidgets import (QMainWindow, QWidget, QLabel, QPushButton, QFrame,
                             QVBoxLayout, QHBoxLayout, QGridLayout, QLineEdit,
                             QListWidget, QAbstractItemView, QSizePolicy,
                             QSpacerItem)
from PyQt5.QtCore import Qt, pyqtSlot

from ..config import ZENOH_CONFIG_PATH, REVERSE_SERVO_MAP, INITIAL_JOINT_STATES
from ..ros_comms import RosNodeThread
from .leg_window import LegDisplayWindow
from .camera_window import CameraDisplayWindow
from .imu_window import IMUDisplayWindow
from .widgets import JoystickWidget, HexapodImageCell

class MainWindow(QMainWindow):
    """The main application window."""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Hexapod Command Node")
        self.setGeometry(100, 100, 1000, 800)
        self.setStyleSheet("QFrame { border: 1px solid #aaa; border-radius: 5px; }")

        self.ros_thread = None
        self.secondary_windows = {}
        self.is_sim_mode = False

        self.master_joint_states = INITIAL_JOINT_STATES.copy()

        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.main_layout = QVBoxLayout(self.central_widget)

        self.main_layout.addWidget(self._create_top_frame())
        self.main_layout.addWidget(self._create_middle_frame(), stretch=1)
        self.main_layout.addWidget(self._create_bottom_frame())

        self._update_connection_status(False)
        self._handle_reconnect()
        
    # ... (No changes to _create_top_frame, _create_middle_frame, etc.) ...
    
    def _create_top_frame(self):
        frame = QFrame()
        frame.setFixedHeight(40)
        layout = QHBoxLayout(frame)
        bold_font = self.font(); bold_font.setBold(True)

        self.conn_status_label = QLabel("Status:")
        self.conn_status_value = QLabel("UNKNOWN")
        self.conn_status_value.setFont(bold_font)
        layout.addWidget(self.conn_status_label)
        layout.addWidget(self.conn_status_value)

        self.mode_toggle_button = QPushButton("Mode: Real Hexapod")
        self.mode_toggle_button.setCheckable(True)
        self.mode_toggle_button.toggled.connect(self._toggle_sim_mode)
        self.mode_toggle_button.setToolTip("Toggle between real robot and Gazebo simulation.")
        layout.addWidget(self.mode_toggle_button)

        layout.addSpacerItem(QSpacerItem(20, 20, QSizePolicy.Fixed, QSizePolicy.Minimum))
        self.ip_address_label = QLabel("Target IP:")
        self.ip_address_input = QLineEdit("localhost")
        self.ip_address_input.setFixedWidth(150)
        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self._handle_reconnect)
        layout.addWidget(self.ip_address_label)
        layout.addWidget(self.ip_address_input)
        layout.addWidget(self.connect_button)

        layout.addStretch()
        self.batt1_level_label = QLabel("Battery1:")
        self.batt1_level_value = QLabel("N/A"); self.batt1_level_value.setFont(bold_font)
        layout.addWidget(self.batt1_level_label)
        layout.addWidget(self.batt1_level_value)
        self.batt2_level_label = QLabel("Battery2:")
        self.batt2_level_value = QLabel("N/A"); self.batt2_level_value.setFont(bold_font)
        layout.addWidget(self.batt2_level_label)
        layout.addWidget(self.batt2_level_value)
        
        return frame

    def _create_middle_frame(self):
        frame = QFrame()
        self.grid_layout = QGridLayout(frame)
        self.grid_layout.setSpacing(10)
        self.grid_layout.setRowStretch(0, 1); self.grid_layout.setRowStretch(6, 1)

        self.leg_value_labels = {f'leg{i}': self._create_label_value_widget(['coxa', 'femur', 'tibia']) for i in range(1, 7)}
        self.cam_value_labels = self._create_label_value_widget(['pan', 'tilt', 'Color', 'Depth'])
        self.imu_value_labels = self._create_label_value_widget(['x accel', 'y accel', 'z accel', 'pitch', 'yaw', 'roll'])

        grid_items = {
            (1, 0): self.leg_value_labels['leg6'], (1, 1): self._create_control_button("Leg 6"),
            (1, 3): self.cam_value_labels, (1, 5): self._create_control_button("Leg 1"),
            (1, 6): self.leg_value_labels['leg1'], (2, 2): HexapodImageCell('front_left'),
            (2, 3): self._create_control_button("Camera"), (2, 4): HexapodImageCell('front_right'),
            (3, 0): self.leg_value_labels['leg5'], (3, 1): self._create_control_button("Leg 5"),
            (3, 2): HexapodImageCell('mid_left'), (3, 3): HexapodImageCell('center_body'),
            (3, 4): HexapodImageCell('mid_right'), (3, 5): self._create_control_button("Leg 2"),
            (3, 6): self.leg_value_labels['leg2'], (4, 2): HexapodImageCell('rear_left'),
            (4, 3): self._create_control_button("IMU"), (4, 4): HexapodImageCell('rear_right'),
            (5, 0): self.leg_value_labels['leg4'], (5, 1): self._create_control_button("Leg 4"),
            (5, 3): self.imu_value_labels, (5, 5): self._create_control_button("Leg 3"),
            (5, 6): self.leg_value_labels['leg3'],
        }
        for pos, widget in grid_items.items():
            self.grid_layout.addWidget(widget, *pos, Qt.AlignCenter)
        
        return frame

    def _create_bottom_frame(self):
        frame = QFrame()
        frame.setFixedHeight(200)
        layout = QHBoxLayout(frame)

        # Joysticks and Lists
        layout.addLayout(self._create_joystick_group("Translational"))
        layout.addLayout(self._create_joystick_group("Rotational"))
        layout.addLayout(self._create_list_group("GAIT", ['Tripod', 'Wave', 'Ripple', 'Tetrapod']))
        layout.addLayout(self._create_list_group("AI Mode", ['Follow', 'Explore', 'Guard', 'Idle']))

        return frame

    def _create_joystick_group(self, title):
        vbox = QVBoxLayout()
        vbox.addWidget(QLabel(f"<b>{title}</b>"), alignment=Qt.AlignCenter)
        vbox.addWidget(JoystickWidget())
        return vbox
    
    def _create_list_group(self, title, items):
        vbox = QVBoxLayout()
        vbox.addWidget(QLabel(f"<b>{title}</b>"), alignment=Qt.AlignCenter)
        list_widget = QListWidget()
        list_widget.setSelectionMode(QAbstractItemView.MultiSelection)
        list_widget.addItems(items)
        vbox.addWidget(list_widget)
        return vbox

    def _create_label_value_widget(self, labels):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        layout.setContentsMargins(0, 0, 0, 0)
        value_labels = {}
        for label_text in labels:
            h_layout = QHBoxLayout()
            h_layout.addWidget(QLabel(f"{label_text.capitalize()}:"))
            value_label = QLabel("N/A")
            value_label.setAlignment(Qt.AlignRight)
            value_labels[label_text.lower()] = value_label
            h_layout.addWidget(value_label)
            layout.addLayout(h_layout)
        widget.value_labels = value_labels
        return widget

    def _create_control_button(self, name):
        button = QPushButton(name)
        button.setMinimumHeight(40)
        if "Leg" in name:
            leg_num = int(name.split(" ")[1])
            button.clicked.connect(lambda: self._open_display_handler("leg", leg_num=leg_num))
        elif "Camera" in name:
            button.clicked.connect(lambda: self._open_display_handler("camera"))
        elif "IMU" in name:
            button.clicked.connect(lambda: self._open_display_handler("imu"))
        return button

    def _toggle_sim_mode(self, checked):
        self.is_sim_mode = checked
        if self.is_sim_mode:
            self.mode_toggle_button.setText("Mode: Gazebo Sim")
            self.ip_address_input.setText("localhost")
        else:
            self.mode_toggle_button.setText("Mode: Real Hexapod")
        if self.ros_thread and self.ros_thread.isRunning():
            self.ros_thread.set_sim_mode(self.is_sim_mode)
        self._update_connection_status("Offline" not in self.conn_status_value.text())

    def _update_zenoh_config(self, ip_address):
        """Writes the provided IP address to the Zenoh config file."""
        config_data = {
            'connect': {'endpoints': [f'udp/{ip_address}:7447', f'tcp/{ip_address}:7447']},
            'timestamping': {'enabled': True, 'drop_future_timestamp': True}
        }
        try:
            config_dir = os.path.dirname(ZENOH_CONFIG_PATH)
            if not os.path.exists(config_dir):
                os.makedirs(config_dir)
            with open(ZENOH_CONFIG_PATH, 'w') as f:
                yaml.dump(config_data, f, default_flow_style=False)
            print(f"Successfully updated Zenoh config with IP: {ip_address}")
            return True
        except (IOError, PermissionError) as e:
            print(f"Error writing to Zenoh config file '{ZENOH_CONFIG_PATH}': {e}")
            return False

    def _handle_reconnect(self):
        print("Attempting to reconnect...")
        self.conn_status_value.setText("Connecting...")
        self.conn_status_value.setStyleSheet("color: #FFC107;") # Yellow

        for window in self.secondary_windows.values(): window.close()
        self.secondary_windows.clear()

        if self.ros_thread and self.ros_thread.isRunning():
            self.ros_thread.telemetry_update_signal.disconnect(self._update_telemetry_display)
            self.ros_thread.stop()
            
        self.master_joint_states = INITIAL_JOINT_STATES.copy()

        ip = self.ip_address_input.text().strip()
        if not ip or not self._update_zenoh_config(ip):
            self.conn_status_value.setText("Config Error" if ip else "Invalid IP")
            self.conn_status_value.setStyleSheet("color: #F44336;")
            return

        self.ros_thread = RosNodeThread()
        self.ros_thread.set_sim_mode(self.is_sim_mode)
        self.ros_thread.telemetry_update_signal.connect(self._update_telemetry_display)
        self.ros_thread.start()
        
    def _open_display_handler(self, win_type, leg_num=None):
        win_id = f"leg_{leg_num}" if win_type == "leg" else win_type
        if win_id in self.secondary_windows and self.secondary_windows[win_id].isVisible():
            self.secondary_windows[win_id].activateWindow()
            return
        
        if not (self.ros_thread and self.ros_thread.isRunning()): return
            
        if win_type == "leg":
            self.secondary_windows[win_id] = LegDisplayWindow(
                f"Leg {leg_num}", self.ros_thread, self.master_joint_states
            )
        elif win_type == "camera":
            self.secondary_windows[win_id] = CameraDisplayWindow(
                self.ros_thread, self.master_joint_states
            )
        elif win_type == "imu": 
            self.secondary_windows[win_id] = IMUDisplayWindow(self.ros_thread)
        
        if win_id in self.secondary_windows:
            self.secondary_windows[win_id].show()

    @pyqtSlot(dict)
    def _update_telemetry_display(self, data):
        if 'connection' in data: self._update_connection_status(data['connection'])
        if 'battery1' in data: self._update_battery_level(self.batt1_level_value, data['battery1'])
        if 'battery2' in data: self._update_battery_level(self.batt2_level_value, data['battery2'])

        if all(k in data for k in ['x', 'y', 'z', 'pitch', 'yaw', 'roll']):
            self.imu_value_labels.value_labels['x accel'].setText(f"{data['x']:.2f}")
            self.imu_value_labels.value_labels['y accel'].setText(f"{data['y']:.2f}")
            self.imu_value_labels.value_labels['z accel'].setText(f"{data['z']:.2f}")
            self.imu_value_labels.value_labels['pitch'].setText(f"{data['pitch']:.2f}°")
            self.imu_value_labels.value_labels['yaw'].setText(f"{data['yaw']:.2f}°")
            self.imu_value_labels.value_labels['roll'].setText(f"{data['roll']:.2f}°")

        for channel, angle in data.items():
            if channel in REVERSE_SERVO_MAP:
                info = REVERSE_SERVO_MAP[channel]
                component, joint = info['component'], info['joint']
                text = f"{angle:.1f}°" if angle != -1 else "Error"
                if component.startswith('leg'):
                    self.leg_value_labels[component].value_labels[joint].setText(text)
                elif component == 'camera':
                    self.cam_value_labels.value_labels[joint].setText(text)

    def _update_connection_status(self, connected):
        mode_str = "Gazebo" if self.is_sim_mode else "Hardware"
        status_text = f"{mode_str} Online" if connected else f"{mode_str} Offline"
        color = "#4CAF50" if connected else "#F44336" # Green or Red
        self.conn_status_value.setText(status_text)
        self.conn_status_value.setStyleSheet(f"color: {color};")

    def _update_battery_level(self, label, level):
        label.setText(f"{level}%")
        color = "#4CAF50" if level > 50 else ("#FFC107" if level > 20 else "#F44336")
        label.setStyleSheet(f"color: {color};")
        
    def closeEvent(self, event):
        print("Closing application...")
        for window in self.secondary_windows.values():
            window.close()
        if self.ros_thread and self.ros_thread.isRunning():
            self.ros_thread.stop()
        event.accept()