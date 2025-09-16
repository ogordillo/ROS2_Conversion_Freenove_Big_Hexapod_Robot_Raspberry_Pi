#include "hexapod_gui_cpp/main_window.hpp"
#include "hexapod_gui_cpp/custom_widgets.hpp" // For HexapodImageCell and JoystickWidget
#include <iostream>
#include <iomanip>

MainWindow::MainWindow(std::shared_ptr<RosNodeWrapper> ros_node) :
    m_main_vbox(Gtk::ORIENTATION_VERTICAL, 5),
    m_ros_node_wrapper(ros_node)
{
    set_title("Hexapod Command Node");
    set_default_size(1000, 800);
    set_border_width(10);

    // Build the UI
    add(m_main_vbox);
    m_main_vbox.pack_start(*create_top_frame(), Gtk::PACK_SHRINK);
    m_main_vbox.pack_start(*create_middle_frame(), Gtk::PACK_EXPAND_WIDGET);
    m_main_vbox.pack_start(*create_bottom_frame(), Gtk::PACK_SHRINK);

    // Connect signals
    m_sim_checkbox.signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::on_sim_checkbox_toggled));
    m_ros_node_wrapper->telemetry_update_signal.connect(sigc::mem_fun(*this, &MainWindow::on_telemetry_update));

    update_connection_status(false);
    show_all_children();
}

MainWindow::~MainWindow() {
    for (auto const& [key, val] : m_secondary_windows) {
        if(val) delete val;
    }
}

bool MainWindow::on_delete_event(GdkEventAny* event) {
    std::cout << "Closing application..." << std::endl;
    // Hide all secondary windows which will trigger their on_delete_event handlers
    for (auto const& [key, window] : m_secondary_windows) {
        if (window && window->is_visible()) {
            window->hide();
        }
    }
    return Gtk::Window::on_delete_event(event);
}


Gtk::Frame* MainWindow::create_top_frame() {
    auto frame = Gtk::make_managed<Gtk::Frame>();
    auto hbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 10);
    hbox->set_border_width(5);
    frame->add(*hbox);

    m_conn_status_label.set_text("Status: ");
    m_conn_status_value.set_markup("<b>UNKNOWN</b>");

    m_sim_checkbox.set_label("Sim");
    m_sim_checkbox.set_tooltip_text("Enable publishing to Gazebo topics");

    m_batt1_label.set_text("Battery1:");
    m_batt1_value.set_markup("<b>N/A</b>");

    m_batt2_label.set_text("Battery2:");
    m_batt2_value.set_markup("<b>N/A</b>");

    hbox->pack_start(m_conn_status_label, Gtk::PACK_SHRINK);
    hbox->pack_start(m_conn_status_value, Gtk::PACK_SHRINK);
    hbox->pack_start(m_sim_checkbox, Gtk::PACK_SHRINK);

    // Spacer
    auto spacer = Gtk::make_managed<Gtk::Box>();
    hbox->pack_start(*spacer, Gtk::PACK_EXPAND_WIDGET);

    hbox->pack_end(m_batt2_value, Gtk::PACK_SHRINK);
    hbox->pack_end(m_batt2_label, Gtk::PACK_SHRINK);
    hbox->pack_end(m_batt1_value, Gtk::PACK_SHRINK);
    hbox->pack_end(m_batt1_label, Gtk::PACK_SHRINK);

    return frame;
}

Gtk::Frame* MainWindow::create_middle_frame() {
    auto frame = Gtk::make_managed<Gtk::Frame>();
    frame->add(m_middle_grid);
    m_middle_grid.set_row_spacing(10);
    m_middle_grid.set_column_spacing(10);
    m_middle_grid.set_halign(Gtk::ALIGN_CENTER);
    m_middle_grid.set_valign(Gtk::ALIGN_CENTER);

    // Create label grids
    m_leg_value_grids["leg1"] = create_label_value_grid({"coxa", "femur", "tibia"}, "leg1");
    m_leg_value_grids["leg2"] = create_label_value_grid({"coxa", "femur", "tibia"}, "leg2");
    m_leg_value_grids["leg3"] = create_label_value_grid({"coxa", "femur", "tibia"}, "leg3");
    m_leg_value_grids["leg4"] = create_label_value_grid({"coxa", "femur", "tibia"}, "leg4");
    m_leg_value_grids["leg5"] = create_label_value_grid({"coxa", "femur", "tibia"}, "leg5");
    m_leg_value_grids["leg6"] = create_label_value_grid({"coxa", "femur", "tibia"}, "leg6");
    m_cam_value_grid = create_label_value_grid({"pan", "tilt"}, "camera");
    m_imu_value_grid = create_label_value_grid({"x accel", "y accel", "z accel", "pitch", "yaw", "roll"}, "imu");

    // Layout buttons and widgets
    auto add_button = [this](const std::string& label, int r, int c, const std::function<void()>& func) {
        auto button = Gtk::make_managed<Gtk::Button>(label);
        button->set_size_request(80, 40);
        button->signal_clicked().connect(func);
        m_middle_grid.attach(*button, c, r, 1, 1);
    };

    m_middle_grid.attach(*m_leg_value_grids["leg6"], 0, 0, 1, 1);
    add_button("Leg 6", 0, 1, [this](){ open_leg_control_window(6); });
    m_middle_grid.attach(*Gtk::make_managed<HexapodImageCell>("front_left"), 2, 0);

    m_middle_grid.attach(*m_leg_value_grids["leg5"], 0, 1, 1, 1);
    add_button("Leg 5", 1, 1, [this](){ open_leg_control_window(5); });
    m_middle_grid.attach(*Gtk::make_managed<HexapodImageCell>("mid_left"), 2, 1);

    m_middle_grid.attach(*m_leg_value_grids["leg4"], 0, 2, 1, 1);
    add_button("Leg 4", 2, 1, [this](){ open_leg_control_window(4); });
    m_middle_grid.attach(*Gtk::make_managed<HexapodImageCell>("rear_left"), 2, 2);

    // Center column
    add_button("Camera", 0, 2, [this](){ open_camera_control_window(); });
    m_middle_grid.attach(*Gtk::make_managed<HexapodImageCell>("center_body"), 2, 1, 2, 1);
    add_button("IMU", 2, 2, [this](){ open_imu_control_window(); });

    m_middle_grid.attach(*m_cam_value_grid, 3, 0);
    m_middle_grid.attach(*m_imu_value_grid, 3, 2);

    // Right column
    m_middle_grid.attach(*Gtk::make_managed<HexapodImageCell>("front_right"), 4, 0);
    add_button("Leg 1", 0, 5, [this](){ open_leg_control_window(1); });
    m_middle_grid.attach(*m_leg_value_grids["leg1"], 6, 0, 1, 1);

    m_middle_grid.attach(*Gtk::make_managed<HexapodImageCell>("mid_right"), 4, 1);
    add_button("Leg 2", 1, 5, [this](){ open_leg_control_window(2); });
    m_middle_grid.attach(*m_leg_value_grids["leg2"], 6, 1, 1, 1);

    m_middle_grid.attach(*Gtk::make_managed<HexapodImageCell>("rear_right"), 4, 2);
    add_button("Leg 3", 2, 5, [this](){ open_leg_control_window(3); });
    m_middle_grid.attach(*m_leg_value_grids["leg3"], 6, 2, 1, 1);

    return frame;
}

Gtk::Frame* MainWindow::create_bottom_frame() {
    auto frame = Gtk::make_managed<Gtk::Frame>();
    auto hbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 20);
    hbox->set_border_width(10);
    frame->add(*hbox);

    // Translational Joystick
    auto trans_vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    trans_vbox->pack_start(*Gtk::make_managed<Gtk::Label>("<b>Translational</b>", Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, true), Gtk::PACK_SHRINK);
    trans_vbox->pack_start(*Gtk::make_managed<JoystickWidget>(), Gtk::PACK_EXPAND_WIDGET);
    hbox->pack_start(*trans_vbox, Gtk::PACK_EXPAND_WIDGET);

    // Rotational Joystick
    auto rot_vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    rot_vbox->pack_start(*Gtk::make_managed<Gtk::Label>("<b>Rotational</b>", Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, true), Gtk::PACK_SHRINK);
    rot_vbox->pack_start(*Gtk::make_managed<JoystickWidget>(), Gtk::PACK_EXPAND_WIDGET);
    hbox->pack_start(*rot_vbox, Gtk::PACK_EXPAND_WIDGET);
    
    // GAIT ListBox
    auto gait_vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    gait_vbox->pack_start(*Gtk::make_managed<Gtk::Label>("<b>GAIT</b>", Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, true), Gtk::PACK_SHRINK);
    auto gait_list = Gtk::make_managed<Gtk::ListBox>();
    gait_list->set_selection_mode(Gtk::SELECTION_MULTIPLE);
    gait_list->add(*Gtk::make_managed<Gtk::Label>("Tripod"));
    gait_list->add(*Gtk::make_managed<Gtk::Label>("Wave"));
    gait_list->add(*Gtk::make_managed<Gtk::Label>("Ripple"));
    gait_list->add(*Gtk::make_managed<Gtk::Label>("Tetrapod"));
    gait_vbox->pack_start(*gait_list, Gtk::PACK_EXPAND_WIDGET);
    hbox->pack_start(*gait_vbox, Gtk::PACK_EXPAND_WIDGET);

    // AI Mode ListBox
    auto ai_vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 5);
    ai_vbox->pack_start(*Gtk::make_managed<Gtk::Label>("<b>AI Mode</b>", Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER, true), Gtk::PACK_SHRINK);
    auto ai_list = Gtk::make_managed<Gtk::ListBox>();
    ai_list->set_selection_mode(Gtk::SELECTION_MULTIPLE);
    ai_list->add(*Gtk::make_managed<Gtk::Label>("Follow"));
    ai_list->add(*Gtk::make_managed<Gtk::Label>("Explore"));
    ai_list->add(*Gtk::make_managed<Gtk::Label>("Guard"));
    ai_list->add(*Gtk::make_managed<Gtk::Label>("Idle"));
    ai_vbox->pack_start(*ai_list, Gtk::PACK_EXPAND_WIDGET);
    hbox->pack_start(*ai_vbox, Gtk::PACK_EXPAND_WIDGET);

    return frame;
}

Gtk::Grid* MainWindow::create_label_value_grid(const std::vector<std::string>& labels, const std::string& component_name) {
    auto grid = Gtk::make_managed<Gtk::Grid>();
    grid->set_column_spacing(5);
    m_value_labels[component_name] = {};
    for (size_t i = 0; i < labels.size(); ++i) {
        std::string label_text = labels[i];
        // Capitalize first letter
        if (!label_text.empty()) {
            label_text[0] = toupper(label_text[0]);
        }
        auto text_label = Gtk::make_managed<Gtk::Label>(label_text + ":");
        auto value_label = Gtk::make_managed<Gtk::Label>("N/A");
        grid->attach(*text_label, 0, i, 1, 1);
        grid->attach(*value_label, 1, i, 1, 1);
        m_value_labels[component_name][labels[i]] = value_label;
    }
    return grid;
}

void MainWindow::on_sim_checkbox_toggled() {
    m_ros_node_wrapper->set_sim_mode(m_sim_checkbox.get_active());
}

void MainWindow::on_telemetry_update(const TelemetryVariant& data) {
    if (std::holds_alternative<bool>(data)) {
        update_connection_status(std::get<bool>(data));
    } else if (std::holds_alternative<std::pair<int, int>>(data)) {
        const auto& batt_data = std::get<std::pair<int, int>>(data);
        update_battery_level(batt_data.first, batt_data.second);
    } else if (std::holds_alternative<ImuData>(data)) {
        const auto& imu = std::get<ImuData>(data);
        std::map<std::string, double> imu_map = {
            {"x accel", imu.x}, {"y accel", imu.y}, {"z accel", imu.z},
            {"pitch", imu.pitch}, {"yaw", imu.yaw}, {"roll", imu.roll}
        };
        for (const auto& pair : imu_map) {
            if (m_value_labels["imu"].count(pair.first)) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2) << pair.second;
                if (pair.first == "pitch" || pair.first == "yaw" || pair.first == "roll") {
                    ss << "°";
                }
                m_value_labels["imu"][pair.first]->set_text(ss.str());
            }
        }
        // Also update the IMU window if it's open
        if (m_secondary_windows.count("imu") && m_secondary_windows["imu"]->is_visible()) {
            static_cast<ImuDisplayWindow*>(m_secondary_windows["imu"])->update_imu_values(imu);
        }

    } else if (std::holds_alternative<std::map<int, double>>(data)) {
        const auto& joint_angles = std::get<std::map<int, double>>(data);
        for (const auto& pair : joint_angles) {
            int channel = pair.first;
            double angle = pair.second;
            if (Constants::REVERSE_SERVO_MAP.count(channel)) {
                const auto& info = Constants::REVERSE_SERVO_MAP.at(channel);
                const std::string& component = info.first;
                const std::string& joint = info.second;
                if (m_value_labels.count(component) && m_value_labels[component].count(joint)) {
                     std::stringstream ss;
                     ss << std::fixed << std::setprecision(1) << angle << "°";
                     m_value_labels[component][joint]->set_text(ss.str());
                }
            }
        }
    }
}

void MainWindow::update_connection_status(bool connected) {
    if (connected) {
        m_conn_status_value.set_markup("<span color='#4CAF50'><b>Connected</b></span>");
    } else {
        m_conn_status_value.set_markup("<span color='#F44336'><b>Disconnected</b></span>");
    }
}

void MainWindow::update_battery_level(int battery_id, int level) {
    std::string color = (level > 50) ? "#4CAF50" : ((level > 20) ? "#FFC107" : "#F44336");
    std::string markup = "<span color='" + color + "'><b>" + std::to_string(level) + "%</b></span>";
    if (battery_id == 1) {
        m_batt1_value.set_markup(markup);
    } else if (battery_id == 2) {
        m_batt2_value.set_markup(markup);
    }
}

void MainWindow::open_leg_control_window(int leg_number) {
    std::string win_id = "leg_" + std::to_string(leg_number);
    if (m_secondary_windows.find(win_id) == m_secondary_windows.end() || !m_secondary_windows[win_id]->is_visible()) {
        if(m_secondary_windows.count(win_id) && m_secondary_windows[win_id]) {
            delete m_secondary_windows[win_id];
        }
        std::string leg_name = "Leg " + std::to_string(leg_number);
        m_secondary_windows[win_id] = new LegControlWindow(leg_name, m_ros_node_wrapper);
        m_secondary_windows[win_id]->show();
    } else {
        m_secondary_windows[win_id]->present();
    }
}

void MainWindow::open_camera_control_window() {
    std::string win_id = "camera";
    if (m_secondary_windows.find(win_id) == m_secondary_windows.end() || !m_secondary_windows[win_id]->is_visible()) {
        if(m_secondary_windows.count(win_id) && m_secondary_windows[win_id]) {
            delete m_secondary_windows[win_id];
        }
        m_secondary_windows[win_id] = new CameraControlWindow(m_ros_node_wrapper);
        m_secondary_windows[win_id]->show();
    } else {
        m_secondary_windows[win_id]->present();
    }
}

void MainWindow::open_imu_control_window() {
    std::string win_id = "imu";
    if (m_secondary_windows.find(win_id) == m_secondary_windows.end() || !m_secondary_windows[win_id]->is_visible()) {
        if(m_secondary_windows.count(win_id) && m_secondary_windows[win_id]) {
            delete m_secondary_windows[win_id];
        }
        m_secondary_windows[win_id] = new ImuDisplayWindow();
        m_secondary_windows[win_id]->show();
    } else {
        m_secondary_windows[win_id]->present();
    }
}