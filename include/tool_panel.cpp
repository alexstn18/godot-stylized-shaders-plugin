#include "tool_panel.hpp"
#include "godot_cpp/variant/color.hpp"
#include "godot_cpp/classes/check_button.hpp"
#include "godot_cpp/classes/directional_light3d.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "outline_shader.hpp"
#include "pixel_shader.hpp"
#include "slider_container.hpp"
#include "util/node_builder.hpp"

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

ToolPanel::ToolPanel() : m_posterize_container(NodeBuilder<SliderContainer>::create()),
                         m_dither_container(NodeBuilder<SliderContainer>::create()),
                         m_outline_container(NodeBuilder<VBoxContainer>::create()),
                         m_crt_container(NodeBuilder<VBoxContainer>::create()),
                         m_pixel_container(NodeBuilder<HBoxContainer>::create())
{
}

ToolPanel::~ToolPanel()
{
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

    /// NodeBuilder object initialization
    // Posterize container
    m_posterize_container
    .slider_container_init("Posterize", "Levels", 
                           1.0, 2.0, 32.0, static_cast<double>(m_cel->get_levels()),
                           callable_mp(m_cel.ptr(), &CelShader::set_levels));
    // Outline container
    m_outline_container.call(&VBoxContainer::set_name, "Outline");
    auto width_container = m_outline_container.add_child<SliderContainer>()
        .slider_container_init("Outline Width", "Outline Width", 
                               0.001, 0.0, 0.01, static_cast<double>(m_outline->get_outline_width()), 
                               callable_mp(m_outline.ptr(), &OutlineShader::set_outline_width));
         
    auto mul_container = m_outline_container.add_child<SliderContainer>()
        .slider_container_init("Outline Width Step", "Outline Width Step", 
                               0.01, 0.01, 1.0, static_cast<double>(m_outline->get_outline_mul()),
                               callable_mp(m_outline.ptr(), &OutlineShader::set_outline_mul));

    m_outline_container.add_child<CheckBox>()
                       .call(&CheckBox::set_text, "Jitter")
                       .call(&CheckBox::connect, "toggled", callable_mp(m_outline.ptr(), &OutlineShader::set_jitter), 0u);

    auto amp_container = m_outline_container.add_child<SliderContainer>()
        .slider_container_init("Jitter Amplitude", "Jitter Amplitude", 
                               0.01, 0.01, 0.1, static_cast<double>(m_outline->get_jitter_amp()), 
                               callable_mp(m_outline.ptr(), &OutlineShader::set_jitter_amp));
    auto freq_container = m_outline_container.add_child<SliderContainer>()
        .slider_container_init("Jitter Frequency", "Jitter Frequency",
                               0.01, 0.01, 0.1, static_cast<double>(m_outline->get_jitter_freq()),
                               callable_mp(m_outline.ptr(), &OutlineShader::set_jitter_freq));
    auto color_container = m_outline_container.add_child<HBoxContainer>();

    color_container.add_child<Label>()
                       .call(&Label::set_text, "Open Color Picker");
    color_container.add_child<ColorPickerButton>()
                       .call(&ColorPickerButton::set_text, "Color Picker Button")
                       .call(&ColorPickerButton::connect, "color_changed", callable_mp(m_outline.ptr(), &OutlineShader::set_outline_color), 0u);
    
    // CRT container
    m_crt_container.call(&VBoxContainer::set_name, "CRT");
    auto curvature_container = m_crt_container.add_child<SliderContainer>()
        .slider_container_init("Curvature", "Curvature", 
                               1.0, 0.0, 10.0, static_cast<double>(m_crt->get_curvature()), 
                               callable_mp(m_crt.ptr(), &CRTShader::set_curvature));
    auto vignette_mul_container = m_crt_container.add_child<SliderContainer>()
        .slider_container_init("Vignette Multiplier", "Vignette Multiplier",
                              1.0, 0.0, 10.0, static_cast<double>(m_crt->get_vignette_mul()), 
                              callable_mp(m_crt.ptr(), &CRTShader::set_vignette_mul));
    auto brightness_container = m_crt_container.add_child<SliderContainer>()
        .slider_container_init("Brightness", "Brightness",
                               0.1, 0.0, 10.0, static_cast<double>(m_crt->get_brightness()),
                               callable_mp(m_crt.ptr(), &CRTShader::set_brightness));
    // Dither container
    m_dither_container
    .slider_container_init("Dither", "Gamma Correction Amount",
                          0.1, 0.0, 10.0, static_cast<double>(m_dither->get_gamma_correction()), 
                          callable_mp(m_dither.ptr(), &DitherShader::set_gamma_correction));
    
    // Pixelize container
    m_pixel_container.call(&HBoxContainer::set_name, "Pixelize");
    auto label = m_pixel_container.add_child<Label>()
         .call(&Label::set_text, "Target Width and Height");

    auto width_spin_box = m_pixel_container.add_child<SpinBox>()
         .call(&SpinBox::set_step, 1.0)
         .call(&SpinBox::set_min, 1.0)
         .call(&SpinBox::set_max, static_cast<double>(DisplayServer::get_singleton()->screen_get_size().x))
         .call(&SpinBox::set_value, static_cast<double>(m_pixel->get_target_width()))
         .call(&SpinBox::connect, "value_changed", callable_mp(m_pixel.ptr(), &PixelShader::set_target_width), 0u);

    auto height_spin_box = m_pixel_container.add_child<SpinBox>()
         .call(&SpinBox::set_step, 1.0)
         .call(&SpinBox::set_min, 1.0)
         .call(&SpinBox::set_max, static_cast<double>(DisplayServer::get_singleton()->screen_get_size().y))
         .call(&SpinBox::set_value, static_cast<double>(m_pixel->get_target_height()))
         .call(&SpinBox::connect, "value_changed", callable_mp(m_pixel.ptr(), &PixelShader::set_target_height), 0u);
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
    m_cel->set_enabled(toggled_on);

    if(toggled_on)
    {
        ADD_EFFECT(m_cel);

        m_tab_container->add_child(m_posterize_container.get());

        UtilityFunctions::print("Posterize toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_cel);
        
        m_tab_container->remove_child(m_posterize_container.get());
        
        UtilityFunctions::print("Posterize toggled off");
        
    }
}

void ToolPanel::_on_outline_toggled(bool toggled_on)
{    
    m_outline->set_enabled(toggled_on);
    if(toggled_on)
    {
        ADD_EFFECT(m_outline);
        
        m_tab_container->add_child(m_outline_container.get());

        UtilityFunctions::print("Outline toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_outline);

        m_tab_container->remove_child(m_outline_container.get());
        
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

    if(toggled_on)
    {
        ADD_EFFECT(m_crt);

        m_tab_container->add_child(m_crt_container.get());

        UtilityFunctions::print("CRT toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_crt);

        m_tab_container->remove_child(m_crt_container.get());

        UtilityFunctions::print("CRT toggled off");
    }
}

void ToolPanel::_on_dither_toggled(bool toggled_on)
{
    m_dither->set_enabled(toggled_on);

    if(toggled_on)
    {
        ADD_EFFECT(m_dither);

        m_tab_container->add_child(m_dither_container.get());

        UtilityFunctions::print("Dither toggled on");
    }
    else 
    {
        REMOVE_EFFECT(m_dither);

        m_tab_container->remove_child(m_dither_container.get());

        UtilityFunctions::print("Dither toggled off");
    }
}

void ToolPanel::_on_pixel_toggled(bool toggled_on)
{
    m_pixel->set_enabled(toggled_on);

    if(toggled_on)
    {
        ADD_EFFECT(m_pixel);

        m_tab_container->add_child(m_pixel_container.get());

        UtilityFunctions::print("Pixelization effect toggled on");
    }
    else
    {
        REMOVE_EFFECT(m_pixel);

        m_tab_container->remove_child(m_pixel_container.get());

        UtilityFunctions::print("Pixelization effect toggled off");
    }
}

void ToolPanel::set_edited_scene_root(Node *edited_scene_root) { m_edited_scene_root = edited_scene_root; }