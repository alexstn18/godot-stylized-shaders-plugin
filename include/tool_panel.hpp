#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/button.hpp>

using namespace godot;

class ToolPanel : public VBoxContainer 
{
    GDCLASS(ToolPanel, VBoxContainer);

private:
    Label *m_label = nullptr;
    HBoxContainer *m_h_box_container = nullptr;
    SpinBox *m_spin_box = nullptr;
    Button *m_apply_button = nullptr;
protected:
    static void _bind_methods();
public:
    ToolPanel() {};
    ~ToolPanel();

    void _ready() override;
    void _on_apply_button_pressed();
};