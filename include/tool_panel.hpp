#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/check_button.hpp>
#include <godot_cpp/classes/item_list.hpp>

using namespace godot;

// Convention:
// Please name the Control derived vars similar to their names as nodes
// e.g. m_cel_toggle == "CelToggle"

class ToolPanel : public VBoxContainer 
{
    GDCLASS(ToolPanel, VBoxContainer);

private:
    OptionButton *m_apply_option_btn = nullptr;
    CheckButton  *m_cel_toggle = nullptr;
    CheckButton  *m_outline_toggle = nullptr;
    CheckButton  *m_invert_toggle = nullptr;
    CheckButton  *m_posterize_toggle = nullptr;
    ItemList     *m_effect_list = nullptr;
protected:
    static void _bind_methods();
public:
    ToolPanel() {};
    ~ToolPanel();

    void _ready() override;
    void _process(double delta) override;
    void _on_posterize_toggled();
};