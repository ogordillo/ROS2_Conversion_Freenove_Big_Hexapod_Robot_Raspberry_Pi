from setuptools import setup, find_packages
from glob import glob
import os

package_name = 'command'

setup(
    name=package_name,
    version='0.0.1',
    # Automatically find the main package and the 'ui' sub-package
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'launch'), glob(os.path.join('launch', '*launch.[pxy][yma]*')))
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Orlando E. Gordillo',
    maintainer_email='ogordillo@miners.utep.edu',
    description='ROS2 node to command the Hexpod robot via windows',
    license='Apache License 2.0',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'command = command.main_gui:main',
        ],
    },
)