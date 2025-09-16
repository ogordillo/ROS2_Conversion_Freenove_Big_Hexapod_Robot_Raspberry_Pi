#include "hexapod_gui_cpp/imu_display_window.hpp"
#include <iomanip>
#include <sstream>

ImuDisplayWindow::ImuDisplayWindow() :
    m_main_vbox(Gtk::ORIENTATION_VERTICAL, 10),
    m_reset_btn("Reset / Calibrate IMU")
{
    set_title("IMU Display");
    set_border_width(10);
    set_default_size(300, -1);
    
    add(m_main_vbox);
    
    m_main_vbox.pack_start(m_reset_btn, Gtk::PACK_SHRINK);
    
    m_grid.set_column_spacing(10);
    m_main_vbox.pack_start(m_grid, Gtk::PACK_SHRINK);
    
    std::vector<std::string> params = {"x accel", "y accel", "z accel", "pitch", "yaw", "roll"};
    for (size_t i = 0; i < params.size(); ++i) {
        std::string cap_name = params[i];
        cap_name[0] = toupper(cap_name[0]);
        auto text_label = Gtk::make_managed<Gtk::Label>(cap_name + ":");
        m_value_labels[params[i]] = Gtk::make_managed<Gtk::Label>("0.00");
        m_grid.attach(*text_label, 0, i, 1, 1);
        m_grid.attach(*m_value_labels[params[i]], 1, i, 1, 1);
    }
    
    m_reset_btn.signal_clicked().connect(sigc::mem_fun(*this, &ImuDisplayWindow::send_imu_reset_command));
    
    show_all_children();
}

ImuDisplayWindow::~ImuDisplayWindow() {}

void ImuDisplayWindow::send_imu_reset_command() {
    // In a real application, this would call a ROS service
    reset_display();
}

void ImuDisplayWindow::reset_display() {
    ImuData zero_data = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    update_imu_values(zero_data);
}


void ImuDisplayWindow::update_imu_values(const ImuData& data) {
    std::map<std::string, double> imu_map = {
        {"x accel", data.x}, {"y accel", data.y}, {"z accel", data.z},
        {"pitch", data.pitch}, {"yaw", data.yaw}, {"roll", data.roll}
    };
    for (const auto& pair : imu_map) {
        if (m_value_labels.count(pair.first)) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << pair.second;
            m_value_labels[pair.first]->set_text(ss.str());
        }
    }
}