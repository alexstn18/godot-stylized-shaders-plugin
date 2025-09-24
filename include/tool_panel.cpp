#include "tool_panel.hpp"
#include "godot_cpp/classes/check_button.hpp"
#include "godot_cpp/classes/directional_light3d.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
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
#include <godot_cpp/classes/h_slider.hpp>
#include <godot_cpp/classes/color_picker.hpp>
#include <godot_cpp/classes/check_box.hpp>

#define CONTROL_QUEUE_FREE(T) if(T) { T->queue_free(); T = nullptr; }

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
    m_cel.instantiate();
    
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
                if(m_camera3d_compositor) 
                {
                    m_camera3d_compositor->set_compositor_effects(m_cmp_arr);
                    if(m_world_environment_compositor) m_world_environment_compositor->set_compositor_effects({});
                }
            }
            else if (selected_idx == m_world_environment_option_index)
            {
                if(m_world_environment_compositor) 
                {
                    m_world_environment_compositor->set_compositor_effects(m_cmp_arr);
                    if(m_camera3d_compositor) m_camera3d_compositor->set_compositor_effects({});
                }
            }
        }
        if(m_outline.is_valid()) m_outline->set_dt(delta);
    }
}

void ToolPanel::_on_cel_toggled(bool toggled_on)
{
    static VBoxContainer *levels_container = nullptr;
    static Label *levels_label = nullptr;
    static HSlider *levels_slider = nullptr;
    
    m_cel->set_enabled(toggled_on);

    if(toggled_on)
    {
        m_cmp_arr.push_back(m_cel);

        levels_container = memnew(VBoxContainer);
        levels_label = memnew(Label);
        levels_slider = memnew(HSlider);

        levels_label->set_text("Levels");
        levels_slider->set_step(1.0);
        levels_slider->set_min(2.0);
        levels_slider->set_max(32.0);
        levels_slider->connect("value_changed", callable_mp(m_cel.ptr(), &CelShader::set_levels));

        levels_container->add_child(levels_label);
        levels_container->add_child(levels_slider);
        add_child(levels_container);

        UtilityFunctions::print("Cel toggled on");
    }
    else 
    {
        m_cmp_arr.pop_back();

        levels_container->remove_child(levels_slider);
        levels_container->remove_child(levels_label);
        remove_child(levels_container);

        CONTROL_QUEUE_FREE(levels_slider);
        CONTROL_QUEUE_FREE(levels_label);
        CONTROL_QUEUE_FREE(levels_container);
        UtilityFunctions::print("Cel toggled off");
        
    }
}

void ToolPanel::_on_outline_toggled(bool toggled_on)
{
    static VBoxContainer *width_container = nullptr;
    static VBoxContainer *mul_container = nullptr;
    static VBoxContainer *amp_container = nullptr;
    static VBoxContainer *freq_container = nullptr;
    static Label *width_label = nullptr;
    static Label *mul_label = nullptr;
    static Label *amp_label = nullptr;
    static Label *freq_label = nullptr;
    static HSlider *width_slider = nullptr;
    static HSlider *mul_slider = nullptr;
    static HSlider *amp_slider = nullptr;
    static HSlider *freq_slider = nullptr;
    static ColorPicker *color_picker = nullptr;
    static CheckBox *check_box = nullptr;
    
    m_outline->set_enabled(toggled_on);
    if(toggled_on)
    {
        m_cmp_arr.push_back(m_outline);
        
        width_container = memnew(VBoxContainer);
        width_label = memnew(Label);
        width_slider = memnew(HSlider);
        mul_container = memnew(VBoxContainer);
        mul_label = memnew(Label);
        mul_slider = memnew(HSlider);
        amp_container = memnew(VBoxContainer);
        amp_label = memnew(Label);
        amp_slider = memnew(HSlider);
        freq_container = memnew(VBoxContainer);
        freq_label = memnew(Label);
        freq_slider = memnew(HSlider);
        color_picker = memnew(ColorPicker);
        check_box = memnew(CheckBox);
        
        width_label->set_text("Outline Width");
        mul_label->set_text("Outline Width Step");
        amp_label->set_text("Jitter Amplitude");
        freq_label->set_text("Jitter Frequency");
        check_box->set_text("Jitter");

        width_slider->set_step(0.001);
        width_slider->set_min(0.0);
        width_slider->set_max(0.01);
        width_slider->connect("value_changed", callable_mp(m_outline.ptr(), &OutlineShader::set_outline_width));
        width_container->add_child(width_label);
        width_container->add_child(width_slider);
        add_child(width_container);

        mul_slider->set_step(0.01);
        mul_slider->set_min(0.01);
        mul_slider->set_max(1.0);
        mul_slider->connect("value_changed", callable_mp(m_outline.ptr(), &OutlineShader::set_outline_mul));
        mul_container->add_child(mul_label);
        mul_container->add_child(mul_slider);
        add_child(mul_container);
        
        check_box->connect("toggled", callable_mp(m_outline.ptr(), &OutlineShader::set_jitter));
        add_child(check_box);

        amp_slider->set_step(0.01);
        amp_slider->set_min(0.01);
        amp_slider->set_max(0.1);
        amp_slider->connect("value_changed", callable_mp(m_outline.ptr(), &OutlineShader::set_jitter_amp));
        amp_container->add_child(amp_label);
        amp_container->add_child(amp_slider);
        add_child(amp_container);
        
        freq_slider->set_step(0.01);
        freq_slider->set_min(0.01);
        freq_slider->set_max(0.1);
        freq_slider->connect("value_changed", callable_mp(m_outline.ptr(), &OutlineShader::set_jitter_freq));
        freq_container->add_child(freq_label);
        freq_container->add_child(freq_slider);
        add_child(freq_container);

        color_picker->connect("color_changed", callable_mp(m_outline.ptr(), &OutlineShader::set_outline_color));
        add_child(color_picker);

        UtilityFunctions::print("Outline toggled on");
    }
    else 
    {
        m_cmp_arr.pop_back();

        width_container->remove_child(width_label);
        width_container->remove_child(width_slider);
        
        mul_container->remove_child(mul_label);
        mul_container->remove_child(mul_slider);
        
        amp_container->remove_child(amp_label);
        amp_container->remove_child(amp_slider);

        freq_container->remove_child(freq_label);
        freq_container->remove_child(freq_slider);

        remove_child(width_container);
        remove_child(mul_container);
        remove_child(amp_container);
        remove_child(freq_container);
        remove_child(check_box);
        remove_child(color_picker);
        
        CONTROL_QUEUE_FREE(width_label);
        CONTROL_QUEUE_FREE(mul_label);
        CONTROL_QUEUE_FREE(width_slider);
        CONTROL_QUEUE_FREE(mul_slider);
        CONTROL_QUEUE_FREE(amp_label);
        CONTROL_QUEUE_FREE(freq_label);
        CONTROL_QUEUE_FREE(amp_slider);
        CONTROL_QUEUE_FREE(freq_slider);
        CONTROL_QUEUE_FREE(check_box);
        CONTROL_QUEUE_FREE(color_picker);
        CONTROL_QUEUE_FREE(width_container);
        CONTROL_QUEUE_FREE(mul_container);
        CONTROL_QUEUE_FREE(amp_container);
        CONTROL_QUEUE_FREE(freq_container);

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

void ToolPanel::set_edited_scene_root(Node *edited_scene_root) { m_edited_scene_root = edited_scene_root; }