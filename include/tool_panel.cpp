#include "tool_panel.hpp"
#include "godot_cpp/classes/check_button.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "outline_shader.hpp"

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/classes/h_slider.hpp>

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
    /// Initialize effects
    m_invert.instantiate();
    m_outline.instantiate();
    
    /// Get UI nodes
    // ApplyToContainer
    m_apply_option_btn = get_node<OptionButton>("ApplyToContainer/OptionButton");
    // ToggleContainer
    m_cel_toggle = get_node<CheckButton>("ToggleContainer/CelToggle");
    m_outline_toggle = get_node<CheckButton>("ToggleContainer/OutlineToggle");
    m_invert_toggle = get_node<CheckButton>("ToggleContainer/InvertToggle");
    m_posterize_toggle = get_node<CheckButton>("ToggleContainer/PosterizeToggle");
    // root
    m_effect_list = get_node<ItemList>("EffectList");
    
    /// Check if "gotten" UI nodes even exist
    ERR_FAIL_COND_MSG(!m_apply_option_btn, "ERROR: Could not find OptionButton node!");
    ERR_FAIL_COND_MSG(!m_cel_toggle, "ERROR: Could not find CelToggle node!");
    ERR_FAIL_COND_MSG(!m_outline_toggle, "ERROR: Could not find OutlineToggle node!");
    ERR_FAIL_COND_MSG(!m_invert_toggle, "ERROR: Could not find InvertToggle node!");
    ERR_FAIL_COND_MSG(!m_posterize_toggle, "ERROR: Could not find PosterizeToggle node!");
    ERR_FAIL_COND_MSG(!m_effect_list, "ERROR: Could not find EffectList node!");

    /// Connect to signals
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
        if(m_apply_option_btn)
        {
            int32_t selected_idx = m_apply_option_btn->get_selected();
            if(selected_idx == m_camera3d_option_index)
            {
                if(m_camera3d_compositor) m_camera3d_compositor->set_compositor_effects(m_cmp_arr);
            }
            else if (selected_idx == m_world_environment_option_index)
            {
                if(m_world_environment_compositor) m_world_environment_compositor->set_compositor_effects(m_cmp_arr);
            }
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
    m_outline->set_enabled(toggled_on);
    static VBoxContainer *width_container = nullptr;
    static VBoxContainer *mul_container = nullptr;
    static Label *width_label = nullptr;
    static Label *mul_label = nullptr;
    static HSlider *width_slider = nullptr;
    static HSlider *mul_slider = nullptr;
    if(toggled_on)
    {
        m_cmp_arr.push_back(m_outline);
        
        width_container = memnew(VBoxContainer);
        width_label = memnew(Label);
        width_label->set_text("Outline Width");
        width_slider = memnew(HSlider);
        width_slider->set_step(0.001);
        width_slider->set_min(0.0);
        width_slider->set_max(0.01);
        width_slider->connect("value_changed", callable_mp(m_outline.ptr(), &OutlineShader::set_outline_width));
        width_container->add_child(width_label);
        width_container->add_child(width_slider);
        add_child(width_container);
        
        mul_container = memnew(VBoxContainer);
        mul_label = memnew(Label);
        mul_label->set_text("Outline Width Step");
        mul_slider = memnew(HSlider);
        mul_slider->set_step(0.01);
        mul_slider->set_min(0.01);
        mul_slider->set_max(1.0);
        mul_slider->connect("value_changed", callable_mp(m_outline.ptr(), &OutlineShader::set_outline_mul));
        mul_container->add_child(mul_label);
        mul_container->add_child(mul_slider);
        add_child(mul_container);
        
        UtilityFunctions::print("Outline toggled on");
    }
    else 
    {
        m_cmp_arr.pop_back();
        width_container->remove_child(width_label);
        width_container->remove_child(width_slider);
        remove_child(width_container);
        mul_container->remove_child(mul_label);
        mul_container->remove_child(mul_slider);
        remove_child(mul_container);
        width_label->queue_free(); width_label = nullptr;
        mul_label->queue_free(); mul_label = nullptr;
        width_slider->queue_free(); width_slider = nullptr;
        mul_slider->queue_free(); mul_slider = nullptr;
        UtilityFunctions::print("Outline toggled off");
    }
}

void ToolPanel::_on_invert_toggled(bool toggled_on)
{
    m_invert->set_enabled(toggled_on);
    if(toggled_on)
    {
        m_cmp_arr.push_back(m_invert);
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