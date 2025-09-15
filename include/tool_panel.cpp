#include "tool_panel.hpp"

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>

void ToolPanel::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_on_apply_button_pressed"), &ToolPanel::_on_apply_button_pressed);
}

ToolPanel::ToolPanel()
{

}

ToolPanel::~ToolPanel()
{
    if(m_label) m_label->queue_free();
    if(m_h_box_container) m_h_box_container->queue_free();
    if(m_spin_box) m_spin_box->queue_free();
    if(m_apply_button) m_apply_button->queue_free();
}

void ToolPanel::_ready()
{
    m_label = get_node<Label>("Label");
    m_h_box_container = get_node<HBoxContainer>("HBoxContainer");
    m_spin_box = get_node<SpinBox>("SpinBox");
    m_apply_button = get_node<Button>("ApplyButton");

    m_apply_button->connect("pressed", Callable(this, "_on_apply_button_pressed"));
}

void ToolPanel::_on_apply_button_pressed()
{
    EditorSelection *es = EditorInterface::get_singleton()->get_selection();
    TypedArray<Node> sn = es->get_selected_nodes(); 
    double angle = m_spin_box->get_value();
    for (const auto &obj : sn) 
    {
        Object *node_obj = obj.get_validated_object();
        if(Node3D *n3d = Object::cast_to<Node3D>(node_obj))
        {
            Vector3 rot = n3d->get_rotation();
            rot.y = UtilityFunctions::randf_range(0, UtilityFunctions::deg_to_rad(angle));

            n3d->set_rotation(rot);
        }
    }

    UtilityFunctions::print("This works!");
}

