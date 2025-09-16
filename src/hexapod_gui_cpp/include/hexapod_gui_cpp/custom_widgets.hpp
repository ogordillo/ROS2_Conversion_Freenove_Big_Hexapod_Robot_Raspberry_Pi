#pragma once

#include <gtkmm/drawingarea.h>
#include <string>
#include <cairomm/context.h>

class JoystickWidget : public Gtk::DrawingArea {
public:
    JoystickWidget();

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
};


class HexapodImageCell : public Gtk::DrawingArea {
public:
    HexapodImageCell(const std::string& segment_type);

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

private:
    std::string m_segment_type;
};