#include "hexapod_gui_cpp/leg_control_window.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

LegControlWindow::LegControlWindow(const std::string& leg_name, std::shared_ptr<RosNodeWrapper> ros_node) :
    m_leg_name(leg_name),
    m_ros_node_wrapper(ros_node),
    m_main_vbox(Gtk::ORIENTATION_VERTICAL, 10),
    m_joint_states(Constants::INITIAL_JOINT_STATES)
{
    set_title(leg_name + " Control");
    set_border_width(10);
    set_default_size(400, -1); // Auto height

    // Extract leg id (e.g., "leg1")
    m_leg_id = m_leg_name;
    m_leg_id.erase(std::remove_if(m_leg_id.begin(), m_leg_id.end(), ::isspace), m_leg_id.end());
    std::transform(m_leg_id.begin(), m_leg_id.end(), m_leg_id.begin(), ::tolower);

    add(m_main_vbox);

    // Joint Control Frame
    auto joint_frame = Gtk::make_managed<Gtk::Frame>("Joint Control (" + leg_name + ")");
    auto joint_grid = Gtk::make_managed<Gtk::Grid>();
    joint_grid->set_border_width(5);
    joint_grid->set_row_spacing(5);
    joint_grid->set_column_spacing(10);
    joint_frame->add(*joint_grid);

    std::vector<std::string> joints = {"coxa", "femur", "tibia"};
    for (size_t i = 0; i < joints.size(); ++i) {
        m_joint_controls[joints[i]] = create_control_triplet(joints[i], 0, 180, *joint_grid, i);
    }
    m_main_vbox.pack_start(*joint_frame, Gtk::PACK_SHRINK);

    // IK Control Frame (Placeholder)
    auto ik_frame = Gtk::make_managed<Gtk::Frame>("Inverse Kinematics (IK) Control");
    auto ik_grid = Gtk::make_managed<Gtk::Grid>();
    ik_grid->set_border_width(5);
    ik_grid->set_row_spacing(5);
    ik_grid->set_column_spacing(10);
    ik_frame->add(*ik_grid);
    std::vector<std::string> ik_params = {"X", "Y", "Z", "Pitch", "Yaw", "Roll"};
    for (size_t i = 0; i < ik_params.size(); ++i) {
        create_control_triplet(ik_params[i], 0, 100, *ik_grid, i);
    }
    m_main_vbox.pack_start(*ik_frame, Gtk::PACK_SHRINK);

    show_all_children();
}

LegControlWindow::~LegControlWindow() {
    for (auto const& [key, val] : m_joint_controls) {
        delete val;
    }
}

LegControlWindow::ControlTriplet* LegControlWindow::create_control_triplet(const std::string& name, double min_val, double max_val, Gtk::Grid& parent_grid, int row) {
    auto triplet = new ControlTriplet();
    std::string cap_name = name;
    cap_name[0] = toupper(cap_name[0]);
    triplet->label.set_text(cap_name);
    triplet->slider.set_range(min_val, max_val);
    triplet->slider.set_value((min_val + max_val) / 2.0);
    triplet->slider.set_hexpand(true);
    triplet->textbox.set_text(std::to_string(triplet->slider.get_value()));
    triplet->textbox.set_width_chars(5);

    parent_grid.attach(triplet->label, 0, row, 1, 1);
    parent_grid.attach(triplet->slider, 1, row, 1, 1);
    parent_grid.attach(triplet->textbox, 2, row, 1, 1);
    
    // Connect signals
    triplet->slider.signal_value_changed().connect(
        [this, &textbox = triplet->textbox, &slider = triplet->slider]() {
            on_slider_changed(&textbox, &slider);
        });
    triplet->textbox.signal_changed().connect(
        [this, &textbox = triplet->textbox, &slider = triplet->slider]() {
            on_textbox_changed(&textbox, &slider);
        });

    if (m_joint_controls.count(name)) { // This is a joint control
        int channel = Constants::SERVO_CHANNELS.at(m_leg_id).at(name);
        triplet->slider.signal_change_value().connect(
            [this, name, channel](Gtk::ScrollType type, double value) {
                this->send_joint_command(name, channel);
                return false; // Let the default handler run
            }, false); // `false` means call our handler *after* the default
        triplet->textbox.signal_activate().connect(
            [this, name, channel]() {
                this->send_joint_command(name, channel);
            });
    }

    return triplet;
}

void LegControlWindow::on_slider_changed(Gtk::Entry* textbox, Gtk::Scale* slider) {
    char buffer[10];
    snprintf(buffer, 10, "%.1f", slider->get_value());
    textbox->set_text(buffer);
}

void LegControlWindow::on_textbox_changed(Gtk::Entry* textbox, Gtk::Scale* slider) {
    try {
        double value = std::stod(textbox->get_text());
        slider->set_value(value);
    } catch (const std::exception& e) {
        // Ignore invalid input
    }
}

void LegControlWindow::send_joint_command(const std::string& joint_name, int channel) {
    int angle = static_cast<int>(m_joint_controls[joint_name]->slider.get_value());
    std::cout << "COMMAND: Leg=" << m_leg_name << ", Joint=" << joint_name
              << ", Channel=" << channel << ", Angle=" << angle << std::endl;
    
    m_ros_node_wrapper->call_set_servo(channel, angle);
    
    double angle_rad = (angle - 90.0) * M_PI / 180.0;
    const std::string& rviz_joint_name = Constants::RVIZ_JOINT_NAMES.at(m_leg_id).at(joint_name);

    m_joint_states[rviz_joint_name] = angle_rad;
    m_ros_node_wrapper->publish_all_joint_states(m_joint_states);
}