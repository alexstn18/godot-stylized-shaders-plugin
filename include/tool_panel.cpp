#include "tool_panel.hpp"
#include "pixel_shader.hpp"
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
#include <godot_cpp/classes/color_picker_button.hpp>
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/h_separator.hpp>
#include <godot_cpp/classes/display_server.hpp>

#define ADD_EFFECT(T) m_effect_arr->add_effect(T);
#define REMOVE_EFFECT(T) m_effect_arr->remove_effect(T);

void ToolPanel::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_on_cel_toggled"), &ToolPanel::_on_cel_toggled);
    ClassDB::bind_method(D_METHOD("_on_outline_toggled"), &ToolPanel::_on_outline_toggled);
    ClassDB::bind_method(D_METHOD("_on_invert_toggled"), &ToolPanel::_on_invert_toggled);
    ClassDB::bind_method(D_METHOD("_on_crt_toggled"), &ToolPanel::_on_crt_toggled);
    ClassDB::bind_method(D_METHOD("_on_dither_toggled"), &ToolPanel::_on_dither_toggled);
    ClassDB::bind_method(D_METHOD("_on_pixel_toggled"), &ToolPanel::_on_pixel_toggled);
}

ToolPanel::~ToolPanel()
{
    CONTROL_QUEUE_FREE(m_apply_option_btn);
    CONTROL_QUEUE_FREE(m_cel_toggle);
    CONTROL_QUEUE_FREE(m_outline_toggle);
    CONTROL_QUEUE_FREE(m_invert_toggle);
    CONTROL_QUEUE_FREE(m_crt_toggle);
    CONTROL_QUEUE_FREE(m_effect_list);
}

void ToolPanel::_ready()
{
    /// Initialize effects
    m_effect_arr.instantiate();
    m_invert.instantiate();
    m_outline.instantiate();
    m_cel.instantiate();
    m_crt.instantiate();
    m_dither.instantiate();
    m_pixel.instantiate();
    
    /// Get UI nodes
    // ApplyToContainer
    m_apply_option_btn = get_node<OptionButton>("ApplyToContainer/OptionButton");
    // ToggleContainer
    m_cel_toggle = get_node<CheckButton>("ToggleContainer/CelToggle");
    m_outline_toggle = get_node<CheckButton>("ToggleContainer/OutlineToggle");
    m_invert_toggle = get_node<CheckButton>("ToggleContainer/InvertToggle");
    m_crt_toggle = get_node<CheckButton>("ToggleContainer/CRTToggle");
    m_dither_toggle = get_node<CheckButton>("ToggleContainer/DitherToggle");
    m_pixel_toggle = get_node<CheckButton>("ToggleContainer/PixelToggle");
    
    // root
    m_effect_list = get_node<ItemList>("EffectList");
    m_tab_container = get_node<TabContainer>("TabContainer");
    
    /// Check if "gotten" UI nodes even exist
    ERR_FAIL_COND_MSG(!m_apply_option_btn, "ERROR: Could not find OptionButton node!");
    ERR_FAIL_COND_MSG(!m_cel_toggle, "ERROR: Could not find CelToggle node!");
    ERR_FAIL_COND_MSG(!m_outline_toggle, "ERROR: Could not find OutlineToggle node!");
    ERR_FAIL_COND_MSG(!m_invert_toggle, "ERROR: Could not find InvertToggle node!");
    ERR_FAIL_COND_MSG(!m_crt_toggle, "ERROR: Could not find CRTToggle node!");
    ERR_FAIL_COND_MSG(!m_dither_toggle, "ERROR: Could not find DitherToggle node!");
    ERR_FAIL_COND_MSG(!m_pixel_toggle, "ERROR: Could not find PixelToggle node!");
    ERR_FAIL_COND_MSG(!m_effect_list, "ERROR: Could not find EffectList node!");
    ERR_FAIL_COND_MSG(!m_tab_container, "ERROR: Could not find TabContainer node!");

    /// Connect to signals
    m_cel_toggle->connect("toggled", Callable(this, "_on_cel_toggled"));
    m_outline_toggle->connect("toggled", Callable(this, "_on_outline_toggled"));
    m_invert_toggle->connect("toggled", Callable(this, "_on_invert_toggled"));
    m_crt_toggle->connect("toggled", Callable(this, "_on_crt_toggled"));
    m_dither_toggle->connect("toggled", Callable(this, "_on_dither_toggled"));
    m_pixel_toggle->connect("toggled", Callable(this, "_on_pixel_toggled"));
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

            // Traverse the current edited scene's children
            for(const auto& child : m_edited_scene_root->get_children())
            {
                Object *child_obj = child.get_validated_object();
                ERR_CONTINUE_MSG(!child_obj, "ERROR: Could not get valid object from node");
    
                // Checking to see if a Camera3D Object is a child in the tree scene, as we need it
                if(Camera3D *c3d = Object::cast_to<Camera3D>(child_obj))
                {
                    m_camera3d = c3d;
                    UtilityFunctions::print("Found Camera3D node in edited scene!");
                }
                
                // Checking to see if a WorldEnvironment Object is a child in the tree scene, as we need it
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
    }
    if(m_outline.is_valid()) m_outline->set_dt(delta);
}

void ToolPanel::_on_cel_toggled(bool toggled_on)
{
    static SliderContainer *container = nullptr;
    
    m_cel->set_enabled(toggled_on);

    if(toggled_on)
    {
        ADD_EFFECT(m_cel);

        container = memnew(SliderContainer);
        container->set_name("Posterize");
        container->set_label_text("Levels");
        container->set_slider_step(1.0);
        container->set_slider_min(2.0);
        container->set_slider_max(32.0);
        container->connect_to_slider(callable_mp(m_cel.ptr(), &CelShader::set_levels));

        m_tab_container->add_child(container);

        UtilityFunctions::print("Cel toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_cel);
        m_tab_container->remove_child(container);
        CONTROL_QUEUE_FREE(container);
        UtilityFunctions::print("Cel toggled off");
        
    }
}

void ToolPanel::_on_outline_toggled(bool toggled_on)
{
    static VBoxContainer     *base_container  = nullptr;
    static SliderContainer   *width_container = nullptr;
    static SliderContainer   *mul_container   = nullptr;
    static SliderContainer   *amp_container   = nullptr;
    static SliderContainer   *freq_container  = nullptr;
    static HSeparator        *width_separator = nullptr;
    static HSeparator        *mul_separator = nullptr;
    static HSeparator        *amp_separator = nullptr;
    static HSeparator        *freq_separator = nullptr;
    static ColorPickerButton *color_picker    = nullptr;
    static CheckBox          *check_box       = nullptr;
    
    m_outline->set_enabled(toggled_on);
    if(toggled_on)
    {
        ADD_EFFECT(m_outline);
        
        base_container = memnew(VBoxContainer);
        width_container = memnew(SliderContainer);
        mul_container = memnew(SliderContainer);
        amp_container = memnew(SliderContainer);
        freq_container = memnew(SliderContainer);
        width_separator = memnew(HSeparator);
        mul_separator  = memnew(HSeparator);
        amp_separator  = memnew(HSeparator);
        freq_separator  = memnew(HSeparator);
        color_picker = memnew(ColorPickerButton);
        check_box = memnew(CheckBox);
        
        base_container->set_name("Outline");

        width_container->set_label_text("Outline Width");
        mul_container->set_label_text("Outline Width Step");
        amp_container->set_label_text("Jitter Amplitude");
        freq_container->set_label_text("Jitter Frequency");
        check_box->set_text("Jitter");

        width_container->set_slider_step(0.001);
        width_container->set_slider_min(0.0);
        width_container->set_slider_max(0.01);
        width_container->connect_to_slider(callable_mp(m_outline.ptr(), &OutlineShader::set_outline_width));
        base_container->add_child(width_container);
        base_container->add_child(width_separator);
        
        mul_container->set_slider_step(0.01);
        mul_container->set_slider_min(0.01);
        mul_container->set_slider_max(1.0);
        mul_container->connect_to_slider(callable_mp(m_outline.ptr(), &OutlineShader::set_outline_mul));
        base_container->add_child(mul_container);
        base_container->add_child(mul_separator);
        
        check_box->connect("toggled", callable_mp(m_outline.ptr(), &OutlineShader::set_jitter));
        base_container->add_child(check_box);
        
        amp_container->set_slider_step(0.01);
        amp_container->set_slider_min(0.01);
        amp_container->set_slider_max(0.1);
        amp_container->connect_to_slider(callable_mp(m_outline.ptr(), &OutlineShader::set_jitter_amp));
        base_container->add_child(amp_container);
        base_container->add_child(amp_separator);
        
        freq_container->set_slider_step(0.01);
        freq_container->set_slider_min(0.01);
        freq_container->set_slider_max(0.1);
        freq_container->connect_to_slider(callable_mp(m_outline.ptr(), &OutlineShader::set_jitter_freq));
        base_container->add_child(freq_container);
        base_container->add_child(freq_separator);
        
        // TODO: Add label here (with the text from below)
        color_picker->set_text("Open Color Picker"); 
        color_picker->connect("color_changed", callable_mp(m_outline.ptr(), &OutlineShader::set_outline_color));
        base_container->add_child(color_picker);
        
        m_tab_container->add_child(base_container);

        UtilityFunctions::print("Outline toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_outline);
        
        base_container->remove_child(width_container);
        base_container->remove_child(mul_container);
        base_container->remove_child(amp_container);
        base_container->remove_child(freq_container);
        base_container->remove_child(width_separator);
        base_container->remove_child(mul_separator);
        base_container->remove_child(amp_separator);
        base_container->remove_child(freq_separator);
        base_container->remove_child(check_box);
        base_container->remove_child(color_picker);
        
        m_tab_container->remove_child(base_container);

        CONTROL_QUEUE_FREE(check_box);
        CONTROL_QUEUE_FREE(color_picker);
        CONTROL_QUEUE_FREE(width_separator);
        CONTROL_QUEUE_FREE(mul_separator);
        CONTROL_QUEUE_FREE(amp_separator);
        CONTROL_QUEUE_FREE(freq_separator);
        CONTROL_QUEUE_FREE(width_container);
        CONTROL_QUEUE_FREE(mul_container);
        CONTROL_QUEUE_FREE(amp_container);
        CONTROL_QUEUE_FREE(freq_container);
        CONTROL_QUEUE_FREE(base_container);
        
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
    
    static VBoxContainer   *base_container         = nullptr;
    static SliderContainer *curvature_container    = nullptr;
    static SliderContainer *vignette_mul_container = nullptr;
    static SliderContainer *brightness_container   = nullptr;

    if(toggled_on)
    {
        ADD_EFFECT(m_crt);
        
        base_container = memnew(VBoxContainer);
        curvature_container = memnew(SliderContainer);
        vignette_mul_container = memnew(SliderContainer);
        brightness_container = memnew(SliderContainer);
        
        base_container->set_name("CRT");

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

        base_container->add_child(curvature_container);
        base_container->add_child(vignette_mul_container);
        base_container->add_child(brightness_container);

        m_tab_container->add_child(base_container);

        UtilityFunctions::print("CRT toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_crt);

        base_container->remove_child(curvature_container);
        base_container->remove_child(vignette_mul_container);
        base_container->remove_child(brightness_container);

        m_tab_container->remove_child(base_container);

        CONTROL_QUEUE_FREE(curvature_container);   
        CONTROL_QUEUE_FREE(vignette_mul_container);
        CONTROL_QUEUE_FREE(brightness_container);  
        CONTROL_QUEUE_FREE(base_container);

        UtilityFunctions::print("CRT toggled off");
    }
}

void ToolPanel::_on_dither_toggled(bool toggled_on)
{
    m_dither->set_enabled(toggled_on);

    static SliderContainer *dither_container = nullptr;

    if(toggled_on)
    {
        ADD_EFFECT(m_dither);

        dither_container = memnew(SliderContainer);
        
        dither_container->set_name("Dither");
        dither_container->set_label_text("Gamma Correction Amount");
        dither_container->set_slider_step(0.1);
        dither_container->set_slider_min(-0.5);
        dither_container->set_slider_max(3.0);
        dither_container->connect_to_slider(callable_mp(m_dither.ptr(), &DitherShader::set_gamma_correction));

        m_tab_container->add_child(dither_container);

        UtilityFunctions::print("Dither toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_dither);

        m_tab_container->remove_child(dither_container);

        CONTROL_QUEUE_FREE(dither_container);

        UtilityFunctions::print("Dither toggled off");
    }
}

void ToolPanel::_on_pixel_toggled(bool toggled_on)
{
    m_pixel->set_enabled(toggled_on);

    static HBoxContainer *resolution_container = nullptr;
    static Label         *label                = nullptr;
    static SpinBox       *width_spin_box       = nullptr;
    static SpinBox       *height_spin_box      = nullptr;

    if(toggled_on)
    {
        ADD_EFFECT(m_pixel);

        resolution_container = memnew(HBoxContainer);
        label = memnew(Label);
        width_spin_box = memnew(SpinBox);
        height_spin_box = memnew(SpinBox);

        label->set_text("Target Width and Height");
        width_spin_box->set_step(1.);
        width_spin_box->set_min(1.);
        width_spin_box->set_max((double)DisplayServer::get_singleton()->screen_get_size().x);
        width_spin_box->connect("value_changed", callable_mp(m_pixel.ptr(), &PixelShader::set_target_width));
        height_spin_box->connect("value_changed", callable_mp(m_pixel.ptr(), &PixelShader::set_target_height));
        height_spin_box->set_step(1.);
        height_spin_box->set_min(1.);
        height_spin_box->set_max((double)DisplayServer::get_singleton()->screen_get_size().y);

        resolution_container->set_name("Pixelize");
        resolution_container->add_child(label);
        resolution_container->add_child(width_spin_box);
        resolution_container->add_child(height_spin_box);
        m_tab_container->add_child(resolution_container);

        UtilityFunctions::print("Pixelization effect toggled on");
    }
    else
    {
        REMOVE_EFFECT(m_pixel);

        resolution_container->remove_child(height_spin_box);
        resolution_container->remove_child(width_spin_box);
        resolution_container->remove_child(label);
        m_tab_container->remove_child(resolution_container);

        CONTROL_QUEUE_FREE(height_spin_box);
        CONTROL_QUEUE_FREE(width_spin_box);
        CONTROL_QUEUE_FREE(label);
        CONTROL_QUEUE_FREE(resolution_container);

        UtilityFunctions::print("Pixelization effect toggled off");
    }
}

void ToolPanel::set_edited_scene_root(Node *edited_scene_root) { m_edited_scene_root = edited_scene_root; }