#include "hexapod_gui_cpp/ros_node_wrapper.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/LinearMath/Matrix3x3.h"

RosNodeWrapper::RosNodeWrapper() {
    // Start the ROS thread
    ros_thread_ = std::thread(&RosNodeWrapper::run, this);
    
    // Connect dispatchers to their handlers
    color_image_dispatcher_.connect(sigc::mem_fun(*this, &RosNodeWrapper::on_color_image_received));
    depth_image_dispatcher_.connect(sigc::mem_fun(*this, &RosNodeWrapper::on_depth_image_received));
    telemetry_dispatcher_.connect(sigc::mem_fun(*this, &RosNodeWrapper::on_telemetry_received));
}

RosNodeWrapper::~RosNodeWrapper() {
    running_ = false;
    if(executor_){
        executor_->cancel();
    }
    if (ros_thread_.joinable()) {
        ros_thread_.join();
    }
}

void RosNodeWrapper::run() {
    node_ = std::make_shared<rclcpp::Node>("hexapod_gui_node");
    executor_ = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
    executor_->add_node(node_);

    set_servo_client_ = node_->create_client<robot_interfaces::srv::SetServo>(Constants::SERVICE_SET_SERVO);
    joint_state_pub_ = node_->create_publisher<sensor_msgs::msg::JointState>(Constants::TOPIC_JOINT_STATES, 10);
    gazebo_joint_pub_ = node_->create_publisher<trajectory_msgs::msg::JointTrajectory>(Constants::TOPIC_GAZEBO_CMD, 10);
    
    joint_state_sub_ = node_->create_subscription<sensor_msgs::msg::JointState>(
        "joint_states", 10, std::bind(&RosNodeWrapper::joint_state_callback, this, std::placeholders::_1));
    battery1_sub_ = node_->create_subscription<sensor_msgs::msg::BatteryState>(
        Constants::TOPIC_BATTERY1, 10, std::bind(&RosNodeWrapper::battery1_callback, this, std::placeholders::_1));
    battery2_sub_ = node_->create_subscription<sensor_msgs::msg::BatteryState>(
        Constants::TOPIC_BATTERY2, 10, std::bind(&RosNodeWrapper::battery2_callback, this, std::placeholders::_1));
    imu_sub_ = node_->create_subscription<sensor_msgs::msg::Imu>(
        Constants::TOPIC_IMU, 10, std::bind(&RosNodeWrapper::imu_callback, this, std::placeholders::_1));

    RCLCPP_INFO(node_->get_logger(), "Hexapod GUI ROS2 Node is running.");
    
    while(rclcpp::ok() && running_){
        executor_->spin_some();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    RCLCPP_INFO(node_->get_logger(), "Hexapod GUI ROS2 Node shutting down.");
}

void RosNodeWrapper::imu_callback(const sensor_msgs::msg::Imu::SharedPtr msg) {
    tf2::Quaternion q(msg->orientation.x, msg->orientation.y, msg->orientation.z, msg->orientation.w);
    tf2::Matrix3x3 m(q);
    double roll, pitch, yaw;
    m.getRPY(roll, pitch, yaw);

    ImuData imu_data = {
        msg->linear_acceleration.x,
        msg->linear_acceleration.y,
        msg->linear_acceleration.z,
        roll * 180.0 / M_PI,
        pitch * 180.0 / M_PI,
        yaw * 180.0 / M_PI
    };
    
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        last_telemetry_data_ = imu_data;
    }
    telemetry_dispatcher_.emit();
    
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        last_telemetry_data_ = true; // Connection is good
    }
    telemetry_dispatcher_.emit();
}

void RosNodeWrapper::battery1_callback(const sensor_msgs::msg::BatteryState::SharedPtr msg) {
    int level = (msg->percentage >= 0) ? static_cast<int>(msg->percentage * 100) : 0;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        last_telemetry_data_ = std::make_pair(1, level);
    }
    telemetry_dispatcher_.emit();
}

void RosNodeWrapper::battery2_callback(const sensor_msgs::msg::BatteryState::SharedPtr msg) {
    int level = (msg->percentage >= 0) ? static_cast<int>(msg->percentage * 100) : 0;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        last_telemetry_data_ = std::make_pair(2, level);
    }
    telemetry_dispatcher_.emit();
}

void RosNodeWrapper::joint_state_callback(const sensor_msgs::msg::JointState::SharedPtr msg) {
    std::map<int, double> telemetry_data;
    for (size_t i = 0; i < msg->name.size(); ++i) {
        const auto& joint_name = msg->name[i];
        if (Constants::JOINT_NAME_TO_CHANNEL_MAP.count(joint_name)) {
            int channel = Constants::JOINT_NAME_TO_CHANNEL_MAP.at(joint_name);
            double angle_deg = (msg->position[i] * 180.0 / M_PI) + 90.0;
            telemetry_data[channel] = angle_deg;
        }
    }

    if (!telemetry_data.empty()) {
        {
            std::lock_guard<std::mutex> lock(data_mutex_);
            last_telemetry_data_ = telemetry_data;
        }
        telemetry_dispatcher_.emit();
    }
}

void RosNodeWrapper::color_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
    try {
        cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, "bgr8");
        {
            std::lock_guard<std::mutex> lock(data_mutex_);
            last_color_image_ = cv_ptr->image;
        }
        color_image_dispatcher_.emit();
    } catch (cv_bridge::Exception& e) {
        RCLCPP_ERROR(node_->get_logger(), "cv_bridge exception: %s", e.what());
    }
}

void RosNodeWrapper::depth_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
    try {
        cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, msg->encoding);
        {
            std::lock_guard<std::mutex> lock(data_mutex_);
            last_depth_image_ = cv_ptr->image;
        }
        depth_image_dispatcher_.emit();
    } catch (cv_bridge::Exception& e) {
        RCLCPP_ERROR(node_->get_logger(), "cv_bridge exception: %s", e.what());
    }
}

void RosNodeWrapper::subscribe_to_video(const std::string& stream_type) {
    if (stream_type == "color" && !color_sub_) {
        color_sub_ = node_->create_subscription<sensor_msgs::msg::Image>(
            Constants::TOPIC_COLOR_IMG, 10, std::bind(&RosNodeWrapper::color_callback, this, std::placeholders::_1));
        RCLCPP_INFO(node_->get_logger(), "Subscribed to %s", Constants::TOPIC_COLOR_IMG.c_str());
    } else if (stream_type == "depth" && !depth_sub_) {
        depth_sub_ = node_->create_subscription<sensor_msgs::msg::Image>(
            Constants::TOPIC_DEPTH_IMG, 10, std::bind(&RosNodeWrapper::depth_callback, this, std::placeholders::_1));
        RCLCPP_INFO(node_->get_logger(), "Subscribed to %s", Constants::TOPIC_DEPTH_IMG.c_str());
    }
}

void RosNodeWrapper::unsubscribe_from_video(const std::string& stream_type) {
    if (stream_type == "color" && color_sub_) {
        color_sub_.reset();
        RCLCPP_INFO(node_->get_logger(), "Unsubscribed from %s", Constants::TOPIC_COLOR_IMG.c_str());
    } else if (stream_type == "depth" && depth_sub_) {
        depth_sub_.reset();
        RCLCPP_INFO(node_->get_logger(), "Unsubscribed from %s", Constants::TOPIC_DEPTH_IMG.c_str());
    }
}

void RosNodeWrapper::call_set_servo(int channel, int angle) {
    if (!set_servo_client_->wait_for_service(std::chrono::seconds(1))) {
        RCLCPP_ERROR(node_->get_logger(), "Service '%s' not available.", Constants::SERVICE_SET_SERVO.c_str());
        {
            std::lock_guard<std::mutex> lock(data_mutex_);
            last_telemetry_data_ = false; // Connection failed
        }
        telemetry_dispatcher_.emit();
        return;
    }
    auto request = std::make_shared<robot_interfaces::srv::SetServo::Request>();
    request->channel = channel;
    request->angle = angle;
    set_servo_client_->async_send_request(request);
    RCLCPP_INFO(node_->get_logger(), "Set servo channel %d to %d degrees.", channel, angle);
}

void RosNodeWrapper::set_sim_mode(bool enabled) {
    sim_mode_.store(enabled);
    if(node_){
        RCLCPP_INFO(node_->get_logger(), "Gazebo simulation publishing is %s.", enabled ? "enabled" : "disabled");
    }
}

void RosNodeWrapper::publish_all_joint_states(const std::map<std::string, double>& joint_states_map) {
    if (!node_) return;
    
    if (sim_mode_.load()) {
        trajectory_msgs::msg::JointTrajectory traj_msg;
        traj_msg.joint_names = Constants::ALL_RVIZ_JOINT_NAMES;
        
        trajectory_msgs::msg::JointTrajectoryPoint point;
        for (const auto& name : Constants::ALL_RVIZ_JOINT_NAMES) {
            point.positions.push_back(joint_states_map.count(name) ? joint_states_map.at(name) : 0.0);
        }
        point.time_from_start.sec = 1;
        point.time_from_start.nanosec = 0;
        
        traj_msg.points.push_back(point);
        gazebo_joint_pub_->publish(traj_msg);
    } else {
        sensor_msgs::msg::JointState rviz_msg;
        rviz_msg.header.stamp = node_->get_clock()->now();
        for(const auto& pair : joint_states_map) {
            rviz_msg.name.push_back(pair.first);
            rviz_msg.position.push_back(pair.second);
        }
        joint_state_pub_->publish(rviz_msg);
    }
}

void RosNodeWrapper::on_color_image_received() {
    cv::Mat image;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        image = last_color_image_.clone();
    }
    if(!image.empty()){
       color_image_signal.emit(image);
    }
}

void RosNodeWrapper::on_depth_image_received() {
    cv::Mat image;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        image = last_depth_image_.clone();
    }
    if(!image.empty()){
        depth_image_signal.emit(image);
    }
}

void RosNodeWrapper::on_telemetry_received() {
    TelemetryVariant data;
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        data = last_telemetry_data_;
    }
    telemetry_update_signal.emit(data);
}