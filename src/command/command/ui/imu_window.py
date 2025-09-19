#!/usr/bin/env python3

"""
IMU display popup window for the Hexapod GUI.
"""

from PyQt5.QtWidgets import QWidget, QLabel, QPushButton, QVBoxLayout, QGridLayout
from PyQt5.QtCore import pyqtSlot

class IMUDisplayWindow(QWidget):
    """A popup window for displaying live IMU data."""
    def __init__(self, ros_node_thread, parent=None):
        super().__init__(parent)
        self.ros_node = ros_node_thread
        self.setWindowTitle("IMU Display")
        self.setMinimumWidth(300)

        layout = QVBoxLayout(self)
        
        grid = QGridLayout()
        self.value_labels = {}
        params = ['x accel', 'y accel', 'z accel', 'pitch', 'yaw', 'roll']
        for i, param in enumerate(params):
            grid.addWidget(QLabel(f"{param.capitalize()}:"), i, 0)
            self.value_labels[param] = QLabel("0.00")
            grid.addWidget(self.value_labels[param], i, 1)
        
        layout.addLayout(grid)
        self.ros_node.telemetry_update_signal.connect(self._handle_telemetry_update)
        self._send_imu_reset_command() # Set initial values to 0

    def _send_imu_reset_command(self):
        print("COMMAND: Reset IMU to all zeros.")
        imu_data = {'x accel': 0, 'y accel': 0, 'z accel': 0, 'pitch': 0, 'yaw': 0, 'roll': 0}
        self._update_imu_values(imu_data)

    @pyqtSlot(dict)
    def _handle_telemetry_update(self, telemetry_data):
        """Receives telemetry and updates the display if IMU data is present."""
        if all(k in telemetry_data for k in ['x', 'y', 'z', 'pitch', 'yaw', 'roll']):
            imu_display_data = {
                'x accel': telemetry_data['x'], 'y accel': telemetry_data['y'],
                'z accel': telemetry_data['z'], 'pitch': telemetry_data['pitch'],
                'yaw': telemetry_data['yaw'], 'roll': telemetry_data['roll']
            }
            self._update_imu_values(imu_display_data)

    def _update_imu_values(self, imu_data):
        for key, value in imu_data.items():
            if key in self.value_labels:
                self.value_labels[key].setText(f"{value:.2f}")

    def closeEvent(self, event):
        """Disconnects the signal on window close to prevent errors."""
        try:
            self.ros_node.telemetry_update_signal.disconnect(self._handle_telemetry_update)
        except TypeError: # Signal might already be disconnected
            pass
        super().closeEvent(event)