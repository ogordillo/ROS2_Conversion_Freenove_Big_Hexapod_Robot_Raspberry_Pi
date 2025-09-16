#include "hexapod_gui_cpp/camera_control_window.hpp"
#include <iostream>
#include <cmath>

CameraControlWindow::CameraControlWindow(std::shared_ptr<RosNodeWrapper> ros_node) :
    m_ros_node_wrapper(ros_node),
    m_main_vbox(Gtk::ORIENTATION_VERTICAL, 10),
    m_joint_states(Constants::INITIAL_JOINT_STATES)
{
    set_title("Camera Control & Display");
    set_border_width(10);
    set_default_size(1300, 600);

    add(m_main_vbox);

    // Servo Control Frame
    auto control_frame = Gtk::make_managed<Gtk::Frame>("Camera Servo Control");
    auto control_grid = Gtk::make_managed<Gtk::Grid>();
    control_grid->set_border_width(5);
    control_grid->set_row_spacing(5);
    control_grid->set_column_spacing(10);
    control_frame->add(*control_grid);

    m_servo_controls["pan"] = create_control_triplet("pan", 0, 180, *control_grid, 0);
    m_servo_controls["tilt"] = create_control_triplet("tilt", 0, 180, *control_grid, 1);
    m_main_vbox.pack_start(*control_frame, Gtk::PACK_SHRINK);

    // Video Frame
    auto video_frame = Gtk::make_managed<Gtk::Frame>("Video Streams");
    auto video_hbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 10);
    video_hbox->set_border_width(5);
    video_frame->add(*video_hbox);

    // Color Stream
    auto color_vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    m_color_view.set_size_request(640, 480);
    m_color_view.set_halign(Gtk::ALIGN_CENTER);
    m_color_view.set_valign(Gtk::ALIGN_CENTER);
    m_color_toggle_btn.set_label("Enable Color Stream");
    color_vbox->pack_start(m_color_view, Gtk::PACK_EXPAND_WIDGET);
    color_vbox->pack_start(m_color_toggle_btn, Gtk::PACK_SHRINK);
    video_hbox->pack_start(*color_vbox, Gtk::PACK_EXPAND_WIDGET);

    // Depth Stream
    auto depth_vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    m_depth_view.set_size_request(640, 480);
    m_depth_view.set_halign(Gtk::ALIGN_CENTER);
    m_depth_view.set_valign(Gtk::ALIGN_CENTER);
    m_depth_toggle_btn.set_label("Enable Depth Stream");
    depth_vbox->pack_start(m_depth_view, Gtk::PACK_EXPAND_WIDGET);
    depth_vbox->pack_start(m_depth_toggle_btn, Gtk::PACK_SHRINK);
    video_hbox->pack_start(*depth_vbox, Gtk::PACK_EXPAND_WIDGET);
    
    m_main_vbox.pack_start(*video_frame, Gtk::PACK_EXPAND_WIDGET);
    
    // Connect signals
    m_color_toggle_btn.signal_toggled().connect([this](){ this->toggle_video_stream("color"); });
    m_depth_toggle_btn.signal_toggled().connect([this](){ this->toggle_video_stream("depth"); });

    m_color_conn = m_ros_node_wrapper->color_image_signal.connect(sigc::mem_fun(*this, &CameraControlWindow::update_color_image));
    m_depth_conn = m_ros_node_wrapper->depth_image_signal.connect(sigc::mem_fun(*this, &CameraControlWindow::update_depth_image));

    show_all_children();
}

CameraControlWindow::~CameraControlWindow() {
     for (auto const& [key, val] : m_servo_controls) {
        delete val;
    }
}

bool CameraControlWindow::on_delete_event(GdkEventAny* event) {
    on_delete_event_override();
    return Gtk::Window::on_delete_event(event);
}

void CameraControlWindow::on_delete_event_override() {
    if (m_color_toggle_btn.get_active()) { m_color_toggle_btn.set_active(false); }
    if (m_depth_toggle_btn.get_active()) { m_depth_toggle_btn.set_active(false); }
    m_color_conn.disconnect();
    m_depth_conn.disconnect();
}


CameraControlWindow::ControlTriplet* CameraControlWindow::create_control_triplet(const std::string& name, double min_val, double max_val, Gtk::Grid& parent_grid, int row) {
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
    
    triplet->slider.signal_value_changed().connect(
        [this, &textbox = triplet->textbox, &slider = triplet->slider]() {
            on_slider_changed(&textbox, &slider);
        });
    triplet->textbox.signal_changed().connect(
        [this, &textbox = triplet->textbox, &slider = triplet->slider]() {
            on_textbox_changed(&textbox, &slider);
        });

    int channel = Constants::SERVO_CHANNELS.at("camera").at(name);
    triplet->slider.signal_change_value().connect(
        [this, name, channel](Gtk::ScrollType type, double value) {
            this->send_camera_command(name, channel);
            return false;
        }, false);
    triplet->textbox.signal_activate().connect(
        [this, name, channel]() {
            this->send_camera_command(name, channel);
        });

    return triplet;
}

void CameraControlWindow::on_slider_changed(Gtk::Entry* textbox, Gtk::Scale* slider) {
    char buffer[10];
    snprintf(buffer, 10, "%.1f", slider->get_value());
    textbox->set_text(buffer);
}

void CameraControlWindow::on_textbox_changed(Gtk::Entry* textbox, Gtk::Scale* slider) {
    try {
        double value = std::stod(textbox->get_text());
        slider->set_value(value);
    } catch (const std::exception& e) {}
}


void CameraControlWindow::send_camera_command(const std::string& servo_name, int channel) {
    int angle = static_cast<int>(m_servo_controls[servo_name]->slider.get_value());
    std::cout << "COMMAND: Camera=" << servo_name << ", Channel=" << channel << ", Angle=" << angle << std::endl;
    
    m_ros_node_wrapper->call_set_servo(channel, angle);
    
    double angle_rad = (angle - 90.0) * M_PI / 180.0;
    const std::string& rviz_joint_name = Constants::RVIZ_JOINT_NAMES.at("camera").at(servo_name);
    
    m_joint_states[rviz_joint_name] = angle_rad;
    m_ros_node_wrapper->publish_all_joint_states(m_joint_states);
}

void CameraControlWindow::toggle_video_stream(const std::string& stream_type) {
    bool checked = (stream_type == "color") ? m_color_toggle_btn.get_active() : m_depth_toggle_btn.get_active();
    Gtk::ToggleButton& btn = (stream_type == "color") ? m_color_toggle_btn : m_depth_toggle_btn;
    Gtk::Image& view = (stream_type == "color") ? m_color_view : m_depth_view;
    std::string cap_type = stream_type;
    cap_type[0] = toupper(cap_type[0]);

    if (checked) {
        m_ros_node_wrapper->subscribe_to_video(stream_type);
        btn.set_label("Disable " + cap_type + " Stream");
    } else {
        m_ros_node_wrapper->unsubscribe_from_video(stream_type);
        btn.set_label("Enable " + cap_type + " Stream");
        view.clear(); // Clear the image
    }
}

void CameraControlWindow::update_color_image(const cv::Mat& cv_image) {
    if (m_color_toggle_btn.get_active() && !cv_image.empty()) {
        m_color_view.set(convert_cv_to_pixbuf(cv_image, 640, 480));
    }
}

void CameraControlWindow::update_depth_image(const cv::Mat& cv_image) {
    if (m_depth_toggle_btn.get_active() && !cv_image.empty()) {
        cv::Mat normalized, colormapped;
        // Assuming 16UC1, typical for depth images
        cv::normalize(cv_image, normalized, 0, 255, cv::NORM_MINMAX, CV_8U);
        cv::applyColorMap(normalized, colormapped, cv::COLORMAP_JET);
        m_depth_view.set(convert_cv_to_pixbuf(colormapped, 640, 480));
    }
}

Glib::RefPtr<Gdk::Pixbuf> CameraControlWindow::convert_cv_to_pixbuf(const cv::Mat& cv_image, int width, int height) {
    int channels = cv_image.channels();
    if (channels != 3 && channels != 1) return nullptr;

    auto pixbuf = Gdk::Pixbuf::create_from_data(
        cv_image.data,
        (channels == 3) ? Gdk::COLORSPACE_RGB : Gdk::COLORSPACE_256,
        channels == 3,
        8,
        cv_image.cols,
        cv_image.rows,
        cv_image.step
    );
    
    // If original is BGR (common in OpenCV), swap channels for RGB pixbuf
    if (channels == 3) {
       return pixbuf->add_alpha(false, '\0', '\0', '\0')->scale_simple(width, height, Gdk::INTERP_BILINEAR);
    }
    return pixbuf->scale_simple(width, height, Gdk::INTERP_BILINEAR);
}