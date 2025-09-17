#include "tool_panel.hpp"
#include "godot_cpp/classes/check_button.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/classes/engine.hpp>

#define CONTROL_QUEUE_FREE(T) if(T) T->queue_free();

void ToolPanel::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_on_cel_toggled"), &ToolPanel::_on_cel_toggled);
    ClassDB::bind_method(D_METHOD("_on_outline_toggled"), &ToolPanel::_on_outline_toggled);
    ClassDB::bind_method(D_METHOD("_on_invert_toggled"), &ToolPanel::_on_invert_toggled);
    ClassDB::bind_method(D_METHOD("_on_posterize_toggled"), &ToolPanel::_on_posterize_toggled);
}

ToolPanel::~ToolPanel()
{
    CONTROL_QUEUE_FREE(m_apply_option_btn);
    CONTROL_QUEUE_FREE(m_cel_toggle);
    CONTROL_QUEUE_FREE(m_outline_toggle);
    CONTROL_QUEUE_FREE(m_invert_toggle);
    CONTROL_QUEUE_FREE(m_posterize_toggle);
    CONTROL_QUEUE_FREE(m_effect_list);
}

void ToolPanel::_ready()
{
    // ApplyToContainer
    m_apply_option_btn = get_node<OptionButton>("ApplyToContainer/OptionButton");
    // ToggleContainer
    m_cel_toggle = get_node<CheckButton>("ToggleContainer/CelToggle");
    m_outline_toggle = get_node<CheckButton>("ToggleContainer/OutlineToggle");
    m_invert_toggle = get_node<CheckButton>("ToggleContainer/InvertToggle");
    m_posterize_toggle = get_node<CheckButton>("ToggleContainer/PosterizeToggle");
    // root
    m_effect_list = get_node<ItemList>("EffectList");
    
    ERR_FAIL_COND_MSG(!m_apply_option_btn, "ERROR: Could not find OptionButton node!");
    ERR_FAIL_COND_MSG(!m_cel_toggle, "ERROR: Could not find CelToggle node!");
    ERR_FAIL_COND_MSG(!m_outline_toggle, "ERROR: Could not find OutlineToggle node!");
    ERR_FAIL_COND_MSG(!m_invert_toggle, "ERROR: Could not find InvertToggle node!");
    ERR_FAIL_COND_MSG(!m_posterize_toggle, "ERROR: Could not find PosterizeToggle node!");
    ERR_FAIL_COND_MSG(!m_effect_list, "ERROR: Could not find EffectList node!");

    m_cel_toggle->connect("toggled", Callable(this, "_on_cel_toggled"));
    m_outline_toggle->connect("toggled", Callable(this, "_on_outline_toggled"));
    m_invert_toggle->connect("toggled", Callable(this, "_on_invert_toggled"));
    m_posterize_toggle->connect("toggled", Callable(this, "_on_posterize_toggled"));
}

void ToolPanel::_process(double delta)
{
    if(Engine::get_singleton()->is_editor_hint())
    {
        // ...
    }
}

void ToolPanel::_on_cel_toggled(bool toggled_on)
{
    if(toggled_on)
    {
        UtilityFunctions::print("Cel toggled on");
        
    }
    else 
    {
        UtilityFunctions::print("Cel toggled off");
    
    }
}

void ToolPanel::_on_outline_toggled(bool toggled_on)
{
    if(toggled_on)
    {
        UtilityFunctions::print("Outline toggled on");
        
    }
    else 
    {
        UtilityFunctions::print("Outline toggled off");
    
    }
}

void ToolPanel::_on_invert_toggled(bool toggled_on)
{
    if(toggled_on)
    {
        UtilityFunctions::print("Invert toggled on");
        
    }
    else 
    {
        UtilityFunctions::print("Invert toggled off");
    
    }
}

void ToolPanel::_on_posterize_toggled(bool toggled_on)
{
    if(toggled_on)
    {
        UtilityFunctions::print("Posterize toggled on");
        
    }
    else 
    {
        UtilityFunctions::print("Posterize toggled off");
    
    }
}