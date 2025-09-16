#include "tool_panel.hpp"

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/object.hpp>

#define CONTROL_QUEUE_FREE(T) if(T) T->queue_free();

// Debug prints (and error checking) have been added thanks to AI!

void ToolPanel::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_on_apply_button_pressed"), &ToolPanel::_on_apply_button_pressed);
}

ToolPanel::~ToolPanel()
{
    CONTROL_QUEUE_FREE(m_label);
    CONTROL_QUEUE_FREE(m_h_box_container);
    CONTROL_QUEUE_FREE(m_spin_box);
    CONTROL_QUEUE_FREE(m_apply_button);
}

void ToolPanel::_ready()
{
    m_label = get_node<Label>("Label");
    if (!m_label) 
    {
        UtilityFunctions::push_error("ERROR: Could not find Label node!");
        return;
    }
    
    m_h_box_container = get_node<HBoxContainer>("HBoxContainer");
    if (!m_h_box_container)
    {
        UtilityFunctions::push_error("ERROR: Could not find HBoxContainer node!");
        return;
    }

    m_spin_box = get_node<SpinBox>("HBoxContainer/SpinBox");
    if (!m_spin_box)
    {
        UtilityFunctions::push_error("ERROR: Could not find SpinBox node!");
        return;
    }
    
    m_apply_button = get_node<Button>("HBoxContainer/Button");
    if (!m_apply_button) 
    {
        UtilityFunctions::push_error("ERROR: Could not find ApplyButton node!");
        return;
    }

    m_apply_button->connect("pressed", Callable(this, "_on_apply_button_pressed"));
}

void ToolPanel::_on_apply_button_pressed()
{
    EditorInterface* editor_interface = EditorInterface::get_singleton();
    if (!editor_interface) 
    {
        UtilityFunctions::push_error("ERROR: EditorInterface singleton is null!");
        return;
    }
    
    EditorSelection *es = editor_interface->get_selection();
    if (!es)
    {
        UtilityFunctions::push_error("ERROR: Could not get EditorSelection!");
        return;
    }
    
    TypedArray<Node> sn = es->get_selected_nodes(); 
    int64_t arr_size = sn.size();
    
    if (arr_size == 0)
    {
        UtilityFunctions::print("WARNING: No nodes selected!");
        return;
    }
    
    if (!m_spin_box)
    {
        UtilityFunctions::push_error("ERROR: SpinBox is null!");
        return;
    }
    
    double angle = m_spin_box->get_value();
    
    for(int i = 0; i < arr_size; i++)
    {
        UtilityFunctions::print("ToolPanel::_on_apply_button_pressed() - Processing node ", i + 1, " of ", arr_size);
        
        Variant var = sn.get(i);
        if (var.get_type() == Variant::NIL)
        {
            UtilityFunctions::push_error("ERROR: Node ", i, " is null variant!");
            continue;
        }
        
        Object *node_obj = var.get_validated_object();
        if (!node_obj)
        {
            UtilityFunctions::push_error("ERROR: Could not get valid object from node ", i);
            continue;
        }
        
        if(Node3D *n3d = Object::cast_to<Node3D>(node_obj))
        {            
            Vector3 rot = n3d->get_rotation();
            
            double angle_rad = UtilityFunctions::deg_to_rad(angle);
            double new_y_rot = UtilityFunctions::randf_range(0, angle_rad);
            rot.y = new_y_rot;
            
            n3d->set_rotation(rot);
            
            Vector3 verify_rot = n3d->get_rotation();
        }
        else
        {
            UtilityFunctions::print("WARNING: Node ", i, " is not a Node3D (Type: ", node_obj->get_class(), ")");
        }
    }
}