cmake_minimum_required(VERSION 3.8)
project(hexapod_gui_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ROS2 and other dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(robot_interfaces REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

# Find GTKmm-3.0 and OpenCV using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(OpenCV REQUIRED opencv4)

# Define the executable
add_executable(hexapod_gui_main
  src/main.cpp
  src/ros_node_wrapper.cpp
  src/main_window.cpp
  src/leg_control_window.cpp
  src/camera_control_window.cpp
  src/imu_display_window.cpp
  src/custom_widgets.cpp
)

# Add include directories
target_include_directories(hexapod_gui_main
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  ${GTKMM_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

# Link libraries
ament_target_dependencies(hexapod_gui_main
  "rclcpp"
  "std_msgs"
  "sensor_msgs"
  "trajectory_msgs"
  "builtin_interfaces"
  "robot_interfaces"
  "cv_bridge"
  "tf2"
  "tf2_geometry_msgs"
)

target_link_libraries(hexapod_gui_main
  ${GTKMM_LIBRARIES}
  ${OpenCV_LIBRARIES}
)

# Install the executable
install(TARGETS
  hexapod_gui_main
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()