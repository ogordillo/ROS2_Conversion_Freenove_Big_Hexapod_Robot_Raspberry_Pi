#!/usr/bin/env python3

"""
Main entry point for the Hexapod Robot Control GUI application.

This script initializes the PyQt5 application and displays the main window.
"""

import sys
from PyQt5.QtWidgets import QApplication

# Import the main window from the UI sub-package
from .ui.main_window import MainWindow

def main():
    """Initializes and runs the PyQt application."""
    app = QApplication(sys.argv)
    app.setStyle('Fusion')
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()