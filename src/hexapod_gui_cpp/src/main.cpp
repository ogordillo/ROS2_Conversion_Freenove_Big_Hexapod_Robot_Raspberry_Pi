#include "rclcpp/rclcpp.hpp"
#include "gtkmm/application.h"
#include "hexapod_gui_cpp/main_window.hpp"
#include "hexapod_gui_cpp/ros_node_wrapper.hpp"

int main(int argc, char *argv[]) {
    // Initialize ROS
    rclcpp::init(argc, argv);
    
    // Create the GTK application
    auto app = Gtk::Application::create(argc, argv, "com.hexapod.gui");

    // Create the ROS node wrapper
    auto ros_node_wrapper = std::make_shared<RosNodeWrapper>();

    // Create the main window, passing the ROS node wrapper
    MainWindow main_window(ros_node_wrapper);

    // Start the GTK main loop
    int result = app->run(main_window);

    // Shutdown ROS
    rclcpp::shutdown();
    
    return result;
}