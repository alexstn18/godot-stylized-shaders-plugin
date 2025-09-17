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
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/environment.hpp>

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
    m_pps.instantiate();
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
        if(m_edited_scene_root)
        {
            if(m_apply_option_btn)
            {
                m_apply_option_btn->clear();
            }
            
            m_camera3d = nullptr;
            m_world_environment = nullptr;
            m_camera3d_option_index = -1;
            m_world_environment_option_index = -1;

            for(const auto& child : m_edited_scene_root->get_children())
            {
                Object *child_obj = child.get_validated_object();
                ERR_CONTINUE_MSG(!child_obj, "ERROR: Could not get valid object from node");
    
                if(Camera3D *c3d = Object::cast_to<Camera3D>(child_obj))
                {
                    m_camera3d = c3d;
                    UtilityFunctions::print("Found Camera3D node in edited scene!");
                }
                
                if(WorldEnvironment *w_env = Object::cast_to<WorldEnvironment>(child_obj))
                {
                    m_world_environment = w_env;
                    UtilityFunctions::print("Found WorldEnvironment node in edited scene!");
                }
            }
    
            if(m_apply_option_btn)
            {
                if(m_camera3d) 
                {
                    Ref<Compositor> c3d_cmp = m_camera3d->get_compositor();
                    Ref<Compositor> wenv_cmp = m_world_environment->get_compositor();
                    if(!c3d_cmp.is_valid()) 
                    {
                        c3d_cmp.instantiate();
                        m_camera3d->set_compositor(c3d_cmp);
                    }
                    if(!wenv_cmp.is_valid())
                    {
                        wenv_cmp.instantiate();
                        m_world_environment->set_compositor(wenv_cmp);
                    } 

                    m_camera3d_compositor = c3d_cmp.ptr();
                    m_world_environment_compositor = wenv_cmp.ptr();

                    ERR_FAIL_COND_MSG(!m_camera3d_compositor, "ERROR: Camera3D compositor invalid!");
                    ERR_FAIL_COND_MSG(!m_world_environment_compositor, "ERROR: WorldEnvironment compositor invalid!");

                    m_camera3d_option_index = m_apply_option_btn->get_item_count();
                    UtilityFunctions::print("Camera3D Option Index: " + String::num(m_camera3d_option_index));
                    m_apply_option_btn->add_item("Camera3D", m_camera3d_option_index);
                }
                if(m_world_environment) 
                {
                    m_world_environment_option_index = m_apply_option_btn->get_item_count();
                    UtilityFunctions::print("WEnv Option Index: " + String::num(m_world_environment_option_index));
                    m_apply_option_btn->add_item("WorldEnvironment", m_world_environment_option_index);
                }
            }

            m_edited_scene_root = nullptr;
        }
        int32_t selected_idx = m_apply_option_btn->get_selected();
        if(selected_idx == m_camera3d_option_index)
        {
            m_camera3d_compositor->set_compositor_effects(m_cmp_arr);
        }
        else if (selected_idx == m_world_environment_option_index)
        {
            m_world_environment_compositor->set_compositor_effects(m_cmp_arr);
        }
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
        m_cmp_arr.push_back(m_pps);
        UtilityFunctions::print("Invert toggled on");        
    }
    else 
    {
        m_cmp_arr.pop_back();
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

void ToolPanel::set_edited_scene_root(Node *edited_scene_root)
{
    m_edited_scene_root = edited_scene_root;
}