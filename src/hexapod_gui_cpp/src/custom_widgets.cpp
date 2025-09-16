#include "hexapod_gui_cpp/custom_widgets.hpp"

// ### JoystickWidget Implementation ###

JoystickWidget::JoystickWidget() {
    set_size_request(100, 100);
}

bool JoystickWidget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    const Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width();
    const int height = allocation.get_height();
    const int size = std::min(width, height);
    const double cx = width / 2.0;
    const double cy = height / 2.0;

    // Draw outer circle
    cr->set_line_width(2.0);
    cr->set_source_rgb(0.47, 0.47, 0.47); // (120, 120, 120)
    cr->arc(cx, cy, size / 2.0 - 5, 0.0, 2.0 * M_PI);
    cr->stroke();

    // Draw inner circle
    cr->set_source_rgb(0.31, 0.31, 0.31); // (80, 80, 80)
    cr->arc(cx, cy, size / 4.0, 0.0, 2.0 * M_PI);
    cr->fill();

    return true;
}

// ### HexapodImageCell Implementation ###

HexapodImageCell::HexapodImageCell(const std::string& segment_type)
    : m_segment_type(segment_type) {
    set_size_request(40, 40);
}

bool HexapodImageCell::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    const Gtk::Allocation allocation = get_allocation();
    const double w = allocation.get_width();
    const double h = allocation.get_height();
    const double cx = w / 2.0;
    const double cy = h / 2.0;

    cr->set_line_width(6.0);
    cr->set_source_rgb(0.39, 0.47, 0.55); // (100, 120, 140)
    cr->set_line_cap(Cairo::LINE_CAP_ROUND);

    if (m_segment_type == "center_body") {
        cr->arc(cx, cy, 15, 0.0, 2.0 * M_PI);
        cr->fill();
    } else if (m_segment_type == "front_right") { cr->move_to(0, h); cr->line_to(cx, cy); cr->stroke(); }
      else if (m_segment_type == "front_left") { cr->move_to(w, h); cr->line_to(cx, cy); cr->stroke(); }
      else if (m_segment_type == "mid_right") { cr->move_to(0, cy); cr->line_to(w, cy); cr->stroke(); }
      else if (m_segment_type == "mid_left") { cr->move_to(w, cy); cr->line_to(0, cy); cr->stroke(); }
      else if (m_segment_type == "rear_right") { cr->move_to(0, 0); cr->line_to(cx, cy); cr->stroke(); }
      else if (m_segment_type == "rear_left") { cr->move_to(w, 0); cr->line_to(cx, cy); cr->stroke(); }

    return true;
}