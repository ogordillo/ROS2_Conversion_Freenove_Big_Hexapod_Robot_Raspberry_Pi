#pragma once

#include <thread>
#include <mutex>
#include <gtkmm.h>
#include <opencv2/opencv.hpp>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "sensor_msgs/msg/battery_state.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "trajectory_msgs/msg/joint_trajectory.hpp"
#include "trajectory_msgs/msg/joint_trajectory_point.hpp"
#include "cv_bridge/cv_bridge.h"
#include "robot_interfaces/srv/set_servo.hpp"
#include "constants.hpp"

// Define a structure to hold all telemetry data
struct TelemetryData {
    std::map<int, double> joint_angles; // channel -> angle_deg
    std::optional<int> battery1_level;
    std::optional<int> battery2_level;
    std::optional<bool> connection_status;
    struct IMU {
        double x, y, z, roll, pitch, yaw;
    };
    std::optional<IMU> imu_data;
};

class RosNodeWrapper {
public:
    RosNodeWrapper();
    ~RosNodeWrapper();

    void set_sim_mode(bool enabled);
    void call_set_servo(int channel, int angle);
    void publish_all_joint_states(const std::map<std::string, double>& joint_states_map);
    void subscribe_to_video(const std::string& stream_type);
    void unsubscribe_from_video(const std::string& stream_type);
    
    // Signals to the GUI thread
    sigc::signal<void, const cv::Mat&> color_image_signal;
    sigc::signal<void, const cv::Mat&> depth_image_signal;
    sigc::signal<void, const TelemetryData&> telemetry_update_signal;

private:
    void run();
    void imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg);
    void battery1_callback(const sensor_msgs::msg::BatteryState::SharedPtr msg);
    void battery2_callback(const sensor_msgs::msg::BatteryState::SharedPtr msg);
    void joint_state_callback(const sensor_msgs::msg::JointState::SharedPtr msg);
    void color_callback(const sensor_msgs::msg::Image::SharedPtr msg);
    void depth_callback(const sensor_msgs::msg::Image::SharedPtr msg);

    // Dispatchers to safely invoke signals from the ROS thread
    void on_color_image_received(const cv::Mat& image);
    void on_depth_image_received(const cv::Mat& image);
    void on_telemetry_received(const TelemetryData& data);

    std::thread ros_thread_;
    std::shared_ptr<rclcpp::Node> node_;
    rclcpp::executors::SingleThreadedExecutor::SharedPtr executor_;
    bool running_ = true;

    // ROS publishers, subscribers, clients
    rclcpp::Client<robot_interfaces::srv::SetServo>::SharedPtr set_servo_client_;
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_state_pub_;
    rclcpp::Publisher<trajectory_msgs::msg::JointTrajectory>::SharedPtr gazebo_joint_pub_;
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr imu_sub_;
    rclcpp::Subscription<sensor_msgs::msg::BatteryState>::SharedPtr battery1_sub_;
    rclcpp::Subscription<sensor_msgs::msg::BatteryState>::SharedPtr battery2_sub_;
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_sub_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr color_sub_;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr depth_sub_;

    std::atomic<bool> sim_mode_{false};
    
    // Thread-safe data handling
    Glib::Dispatcher color_image_dispatcher_;
    Glib::Dispatcher depth_image_dispatcher_;
    Glib::Dispatcher telemetry_dispatcher_;
    
    std::mutex cv_mat_mutex_;
    cv::Mat last_color_image_;
    cv::Mat last_depth_image_;

    std::mutex telemetry_mutex_;
    TelemetryData last_telemetry_data_;
};