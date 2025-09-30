#include "tool_panel.hpp"
#include "psx_shader.hpp"
#include "slider_container.hpp"
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

#define ADD_EFFECT(T) m_effect_arr->add_effect(T);
#define REMOVE_EFFECT(T) m_effect_arr->remove_effect(T);

void ToolPanel::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_on_cel_toggled"), &ToolPanel::_on_cel_toggled);
    ClassDB::bind_method(D_METHOD("_on_outline_toggled"), &ToolPanel::_on_outline_toggled);
    ClassDB::bind_method(D_METHOD("_on_invert_toggled"), &ToolPanel::_on_invert_toggled);
    ClassDB::bind_method(D_METHOD("_on_crt_toggled"), &ToolPanel::_on_crt_toggled);
    ClassDB::bind_method(D_METHOD("_on_psx_toggled"), &ToolPanel::_on_psx_toggled);
}

ToolPanel::~ToolPanel()
{
    remove_child(m_inspector);

    CONTROL_QUEUE_FREE(m_apply_option_btn);
    CONTROL_QUEUE_FREE(m_cel_toggle);
    CONTROL_QUEUE_FREE(m_outline_toggle);
    CONTROL_QUEUE_FREE(m_invert_toggle);
    CONTROL_QUEUE_FREE(m_crt_toggle);
    CONTROL_QUEUE_FREE(m_effect_list);
    CONTROL_QUEUE_FREE(m_inspector);
}

void ToolPanel::_ready()
{
    /// Initialize effects
    m_effect_arr.instantiate();
    m_invert.instantiate();
    m_outline.instantiate();
    m_cel.instantiate();
    m_crt.instantiate();
    m_psx.instantiate();
    
    /// Get UI nodes
    // ApplyToContainer
    m_apply_option_btn = get_node<OptionButton>("ApplyToContainer/OptionButton");
    // ToggleContainer
    m_cel_toggle = get_node<CheckButton>("ToggleContainer/CelToggle");
    m_outline_toggle = get_node<CheckButton>("ToggleContainer/OutlineToggle");
    m_invert_toggle = get_node<CheckButton>("ToggleContainer/InvertToggle");
    m_crt_toggle = get_node<CheckButton>("ToggleContainer/CRTToggle");
    m_psx_toggle = get_node<CheckButton>("ToggleContainer/PSXToggle");
    
    // root
    m_effect_list = get_node<ItemList>("EffectList");
    
    /// Check if "gotten" UI nodes even exist
    ERR_FAIL_COND_MSG(!m_apply_option_btn, "ERROR: Could not find OptionButton node!");
    ERR_FAIL_COND_MSG(!m_cel_toggle, "ERROR: Could not find CelToggle node!");
    ERR_FAIL_COND_MSG(!m_outline_toggle, "ERROR: Could not find OutlineToggle node!");
    ERR_FAIL_COND_MSG(!m_invert_toggle, "ERROR: Could not find InvertToggle node!");
    ERR_FAIL_COND_MSG(!m_crt_toggle, "ERROR: Could not find CRTToggle node!");
    ERR_FAIL_COND_MSG(!m_psx_toggle, "ERROR: Could not find PSXToggle node!");
    ERR_FAIL_COND_MSG(!m_effect_list, "ERROR: Could not find EffectList node!");

    /// Connect to signals
    m_cel_toggle->connect("toggled", Callable(this, "_on_cel_toggled"));
    m_outline_toggle->connect("toggled", Callable(this, "_on_outline_toggled"));
    m_invert_toggle->connect("toggled", Callable(this, "_on_invert_toggled"));
    m_crt_toggle->connect("toggled", Callable(this, "_on_crt_toggled"));
    m_psx_toggle->connect("toggled", Callable(this, "_on_psx_toggled"));

    m_inspector = memnew(EditorInspector);
    m_inspector->edit(m_effect_arr.ptr());
    add_child(m_inspector);
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
                    m_camera3d_compositor->set_compositor_effects(m_effect_arr->get_effects());
                    if(m_world_environment_compositor) m_world_environment_compositor->set_compositor_effects({});
                }
            }
            else if (selected_idx == m_world_environment_option_index)
            {
                if(m_world_environment_compositor) 
                {
                    m_world_environment_compositor->set_compositor_effects(m_effect_arr->get_effects());
                    if(m_camera3d_compositor) m_camera3d_compositor->set_compositor_effects({});
                }
            }
        }
        if(m_outline.is_valid()) m_outline->set_dt(delta);
    }
}

void ToolPanel::_on_cel_toggled(bool toggled_on)
{
    static SliderContainer *container = nullptr;
    
    m_cel->set_enabled(toggled_on);

    if(toggled_on)
    {
        ADD_EFFECT(m_cel);

        container = memnew(SliderContainer);
        container->set_label_text("Levels");
        container->set_slider_step(1.0);
        container->set_slider_min(2.0);
        container->set_slider_max(32.0);
        container->connect_to_slider(callable_mp(m_cel.ptr(), &CelShader::set_levels));

        add_child(container);

        UtilityFunctions::print("Cel toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_cel);
        remove_child(container);
        CONTROL_QUEUE_FREE(container);
        UtilityFunctions::print("Cel toggled off");
        
    }
}

void ToolPanel::_on_outline_toggled(bool toggled_on)
{
    static SliderContainer *width_container = nullptr;
    static SliderContainer *mul_container = nullptr;
    static SliderContainer *amp_container = nullptr;
    static SliderContainer *freq_container = nullptr;
    static ColorPicker *color_picker = nullptr;
    static CheckBox *check_box = nullptr;
    
    m_outline->set_enabled(toggled_on);
    if(toggled_on)
    {
        ADD_EFFECT(m_outline);
        
        width_container = memnew(SliderContainer);
        mul_container = memnew(SliderContainer);
        amp_container = memnew(SliderContainer);
        freq_container = memnew(SliderContainer);
        color_picker = memnew(ColorPicker);
        check_box = memnew(CheckBox);
        
        width_container->set_label_text("Outline Width");
        mul_container->set_label_text("Outline Width Step");
        amp_container->set_label_text("Jitter Amplitude");
        freq_container->set_label_text("Jitter Frequency");
        check_box->set_text("Jitter");

        width_container->set_slider_step(0.001);
        width_container->set_slider_min(0.0);
        width_container->set_slider_max(0.01);
        width_container->connect_to_slider(callable_mp(m_outline.ptr(), &OutlineShader::set_outline_width));
        add_child(width_container);

        mul_container->set_slider_step(0.01);
        mul_container->set_slider_min(0.01);
        mul_container->set_slider_max(1.0);
        mul_container->connect_to_slider(callable_mp(m_outline.ptr(), &OutlineShader::set_outline_mul));
        add_child(mul_container);
        
        check_box->connect("toggled", callable_mp(m_outline.ptr(), &OutlineShader::set_jitter));
        add_child(check_box);

        amp_container->set_slider_step(0.01);
        amp_container->set_slider_min(0.01);
        amp_container->set_slider_max(0.1);
        amp_container->connect_to_slider(callable_mp(m_outline.ptr(), &OutlineShader::set_jitter_amp));
        add_child(amp_container);
        
        freq_container->set_slider_step(0.01);
        freq_container->set_slider_min(0.01);
        freq_container->set_slider_max(0.1);
        freq_container->connect_to_slider(callable_mp(m_outline.ptr(), &OutlineShader::set_jitter_freq));
        add_child(freq_container);
        
        color_picker->connect("color_changed", callable_mp(m_outline.ptr(), &OutlineShader::set_outline_color));
        add_child(color_picker);
        
        UtilityFunctions::print("Outline toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_outline);
        
        remove_child(width_container);
        remove_child(mul_container);
        remove_child(amp_container);
        remove_child(freq_container);
        remove_child(check_box);
        remove_child(color_picker);
        
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
        ADD_EFFECT(m_invert);
        UtilityFunctions::print("Invert toggled on");        
    }
    else 
    {
        REMOVE_EFFECT(m_invert);
        UtilityFunctions::print("Invert toggled off");
    }
}

void ToolPanel::_on_crt_toggled(bool toggled_on)
{
    m_crt->set_enabled(toggled_on);
    
    static SliderContainer *curvature_container    = nullptr;
    static SliderContainer *vignette_mul_container = nullptr;
    static SliderContainer *brightness_container   = nullptr;

    if(toggled_on)
    {
        ADD_EFFECT(m_crt);
        
        curvature_container = memnew(SliderContainer);
        vignette_mul_container = memnew(SliderContainer);
        brightness_container = memnew(SliderContainer);
        
        curvature_container->set_label_text("Curvature");
        vignette_mul_container->set_label_text("Vignette Multiplier");
        brightness_container->set_label_text("Brightness");
        
        curvature_container->set_slider_step(1.0);
        curvature_container->set_slider_min(0.0);
        curvature_container->set_slider_max(10.0);
        curvature_container->connect_to_slider(callable_mp(m_crt.ptr(), &CRTShader::set_curvature));
        
        vignette_mul_container->set_slider_step(1.0);
        vignette_mul_container->set_slider_min(0.0);
        vignette_mul_container->set_slider_max(10.0);
        vignette_mul_container->connect_to_slider(callable_mp(m_crt.ptr(), &CRTShader::set_vignette_mul));
        
        brightness_container->set_slider_step(0.1);
        brightness_container->set_slider_min(0.0);
        brightness_container->set_slider_max(10.0);
        brightness_container->connect_to_slider(callable_mp(m_crt.ptr(), &CRTShader::set_brightness));

        add_child(curvature_container);
        add_child(vignette_mul_container);
        add_child(brightness_container);

        UtilityFunctions::print("CRT toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_crt);

        remove_child(curvature_container);
        remove_child(vignette_mul_container);
        remove_child(brightness_container);

        CONTROL_QUEUE_FREE(curvature_container);   
        CONTROL_QUEUE_FREE(vignette_mul_container);
        CONTROL_QUEUE_FREE(brightness_container);  

        UtilityFunctions::print("CRT toggled off");
    }
}

void ToolPanel::_on_psx_toggled(bool toggled_on)
{
    m_psx->set_enabled(toggled_on);

    static SliderContainer *dither_container = nullptr;

    if(toggled_on)
    {
        ADD_EFFECT(m_psx);

        dither_container = memnew(SliderContainer);
        
        dither_container->set_label_text("Dither Amount");
        dither_container->set_slider_step(0.1);
        dither_container->set_slider_min(0.0);
        dither_container->set_slider_max(1.0);
        dither_container->connect_to_slider(callable_mp(m_psx.ptr(), &PSXShader::set_dither_amount));

        add_child(dither_container);

        UtilityFunctions::print("PSX toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_psx);

        remove_child(dither_container);

        CONTROL_QUEUE_FREE(dither_container);

        UtilityFunctions::print("PSX toggled off");
    }
}

void ToolPanel::set_edited_scene_root(Node *edited_scene_root) { m_edited_scene_root = edited_scene_root; }