#!/usr/bin/env python3

"""
Custom reusable PyQt5 widgets for the Hexapod GUI.
"""

from PyQt5.QtWidgets import QWidget, QLabel, QSizePolicy
from PyQt5.QtGui import QPainter, QPen, QColor, QBrush
from PyQt5.QtCore import Qt

class JoystickWidget(QWidget):
    """ A simple visual placeholder for a joystick. """
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumSize(100, 100)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        size = min(self.width(), self.height())
        painter.setPen(QPen(QColor(120, 120, 120), 2))
        painter.drawEllipse(self.rect().center(), size // 2 - 5, size // 2 - 5)
        painter.setBrush(QBrush(QColor(80, 80, 80)))
        painter.drawEllipse(self.rect().center(), size // 4, size // 4)

class HexapodImageCell(QLabel):
    """ A QLabel that draws a segment of the hexapod body. """
    def __init__(self, segment_type, parent=None):
        super().__init__(parent)
        self.segment_type = segment_type
        self.setMinimumSize(40, 40)
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

    def paintEvent(self, event):
        super().paintEvent(event)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        pen = QPen(QColor(100, 120, 140), 6, Qt.SolidLine, Qt.RoundCap)
        painter.setPen(pen)
        w, h = self.width(), self.height()
        cx, cy = w // 2, h // 2
        if self.segment_type == 'center_body':
            painter.setBrush(QBrush(QColor(100, 120, 140)))
            painter.drawEllipse(cx - 15, cy - 15, 30, 30)
        elif self.segment_type == 'front_right': painter.drawLine(0, h, cx, cy)
        elif self.segment_type == 'front_left':  painter.drawLine(w, h, cx, cy)
        elif self.segment_type == 'mid_right':   painter.drawLine(0, cy, w, cy)
        elif self.segment_type == 'mid_left':    painter.drawLine(w, cy, 0, cy)
        elif self.segment_type == 'rear_right':  painter.drawLine(0, 0, cx, cy)
        elif self.segment_type == 'rear_left':   painter.drawLine(w, 0, cx, cy)