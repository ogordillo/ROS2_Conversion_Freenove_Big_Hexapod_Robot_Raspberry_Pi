#pragma once

#include <gtkmm.h>
#include <map>
#include <string>
#include "ros_node_wrapper.hpp"

class ImuDisplayWindow : public Gtk::Window {
public:
    ImuDisplayWindow();
    virtual ~ImuDisplayWindow();

    void update_imu_values(const ImuData& data);

private:
    void send_imu_reset_command();
    void reset_display();
    
    Gtk::Box m_main_vbox;
    Gtk::Grid m_grid;
    Gtk::Button m_reset_btn;
    std::map<std::string, Gtk::Label*> m_value_labels;
};