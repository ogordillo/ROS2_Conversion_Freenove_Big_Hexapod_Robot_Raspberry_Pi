#pragma once

#include <gtkmm.h>
#include <string>
#include <memory>
#include <map>
#include "ros_node_wrapper.hpp"

class LegControlWindow : public Gtk::Window {
public:
    LegControlWindow(const std::string& leg_name, std::shared_ptr<RosNodeWrapper> ros_node);
    virtual ~LegControlWindow();

private:
    struct ControlTriplet {
        Gtk::Label label;
        Gtk::Scale slider;
        Gtk::Entry textbox;
    };

    void send_joint_command(const std::string& joint_name, int channel);
    ControlTriplet* create_control_triplet(const std::string& name, double min_val, double max_val, Gtk::Grid& parent_grid, int row);
    void on_slider_changed(Gtk::Entry* textbox, Gtk::Scale* slider);
    void on_textbox_changed(Gtk::Entry* textbox, Gtk::Scale* slider);

    std::string m_leg_name;
    std::string m_leg_id; // "leg1", "leg2", etc.
    std::shared_ptr<RosNodeWrapper> m_ros_node_wrapper;

    Gtk::Box m_main_vbox;
    std::map<std::string, ControlTriplet*> m_joint_controls;
    
    // Store all joint states for publishing
    std::map<std::string, double> m_joint_states;
};