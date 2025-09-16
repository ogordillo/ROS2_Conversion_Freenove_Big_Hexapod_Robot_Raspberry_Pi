#pragma once

#include <gtkmm.h>
#include <memory>
#include <map>
#include "ros_node_wrapper.hpp"

class CameraControlWindow : public Gtk::Window {
public:
    CameraControlWindow(std::shared_ptr<RosNodeWrapper> ros_node);
    virtual ~CameraControlWindow();

protected:
    void on_delete_event_override();
    bool on_delete_event(GdkEventAny* event) override;
private:
    struct ControlTriplet {
        Gtk::Label label;
        Gtk::Scale slider;
        Gtk::Entry textbox;
    };

    void send_camera_command(const std::string& servo_name, int channel);
    void toggle_video_stream(const std::string& stream_type);

    ControlTriplet* create_control_triplet(const std::string& name, double min_val, double max_val, Gtk::Grid& parent_grid, int row);
    void on_slider_changed(Gtk::Entry* textbox, Gtk::Scale* slider);
    void on_textbox_changed(Gtk::Entry* textbox, Gtk::Scale* slider);

    void update_color_image(const cv::Mat& cv_image);
    void update_depth_image(const cv::Mat& cv_image);
    Glib::RefPtr<Gdk::Pixbuf> convert_cv_to_pixbuf(const cv::Mat& cv_image, int width, int height);
    
    std::shared_ptr<RosNodeWrapper> m_ros_node_wrapper;
    std::map<std::string, double> m_joint_states;
    
    Gtk::Box m_main_vbox;
    Gtk::Image m_color_view, m_depth_view;
    Gtk::ToggleButton m_color_toggle_btn, m_depth_toggle_btn;

    std::map<std::string, ControlTriplet*> m_servo_controls;

    sigc::connection m_color_conn, m_depth_conn;
};