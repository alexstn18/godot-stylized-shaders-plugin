#pragma once
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/h_slider.hpp>
#include <godot_cpp/variant/callable.hpp>

using namespace godot;

#define CONTROL_QUEUE_FREE(T) if(T) { T->queue_free(); T = nullptr; }

class SliderContainer : public VBoxContainer
{
    GDCLASS(SliderContainer, VBoxContainer);

private:
    Label *m_label = nullptr;
    HSlider *m_slider = nullptr;
protected:
    static void _bind_methods();
public:
    SliderContainer();
    ~SliderContainer();

    void set_label_text(const String &text);

    void connect_to_slider(const Callable &c);
    void set_slider_step(const double step);
    void set_slider_min(const double min);
    void set_slider_max(const double max);
};