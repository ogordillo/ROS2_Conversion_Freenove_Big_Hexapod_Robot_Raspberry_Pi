#pragma once

#include <gtkmm.h>
#include <map>
#include "ros_node_wrapper.hpp"
#include "leg_control_window.hpp"
#include "camera_control_window.hpp"
#include "imu_display_window.hpp"

// Forward declaration
class HexapodImageCell;

class MainWindow : public Gtk::Window {
public:
    MainWindow(std::shared_ptr<RosNodeWrapper> ros_node);
    virtual ~MainWindow();

protected:
    // Signal handlers
    void on_sim_checkbox_toggled();
    void on_telemetry_update(const TelemetryVariant& data);
    void open_leg_control_window(int leg_number);
    void open_camera_control_window();
    void open_imu_control_window();
    bool on_delete_event(GdkEventAny* event) override;

private:
    Gtk::Box m_main_vbox;
    Gtk::Grid m_middle_grid;

    // Top Frame Widgets
    Gtk::Label m_conn_status_label, m_conn_status_value;
    Gtk::Label m_batt1_label, m_batt1_value;
    Gtk::Label m_batt2_label, m_batt2_value;
    Gtk::CheckButton m_sim_checkbox;

    // Middle Frame Widgets
    std::map<std::string, Gtk::Grid*> m_leg_value_grids;
    Gtk::Grid* m_cam_value_grid;
    Gtk::Grid* m_imu_value_grid;
    std::map<std::string, std::map<std::string, Gtk::Label*>> m_value_labels;

    // Helper functions to create widgets
    Gtk::Frame* create_top_frame();
    Gtk::Frame* create_middle_frame();
    Gtk::Frame* create_bottom_frame();
    Gtk::Grid* create_label_value_grid(const std::vector<std::string>& labels, const std::string& component_name);

    void update_connection_status(bool connected);
    void update_battery_level(int battery_id, int level);

    std::shared_ptr<RosNodeWrapper> m_ros_node_wrapper;
    
    // Pointers to secondary windows
    std::map<std::string, Gtk::Window*> m_secondary_windows;
};