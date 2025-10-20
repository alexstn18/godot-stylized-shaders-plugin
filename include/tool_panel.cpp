#include "tool_panel.hpp"
#include "ext/callable_lambda.hpp"
#include "godot_cpp/classes/check_button.hpp"
#include "godot_cpp/classes/directional_light3d.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/color.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "outline_shader.hpp"
#include "pixel_shader.hpp"
#include "slider_container.hpp"
#include "util/encapsulated_data.hpp"
#include "util/node_builder.hpp"

#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/color_picker_button.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_selection.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/classes/h_slider.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/variant/callable.hpp>

#define ADD_EFFECT(T) m_effect_arr->add_effect(T);
#define REMOVE_EFFECT(T) m_effect_arr->remove_effect(T);

void ToolPanel::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_on_cel_toggled"),
                         &ToolPanel::_on_cel_toggled);
    ClassDB::bind_method(D_METHOD("_on_outline_toggled"),
                         &ToolPanel::_on_outline_toggled);
    ClassDB::bind_method(D_METHOD("_on_invert_toggled"),
                         &ToolPanel::_on_invert_toggled);
    ClassDB::bind_method(D_METHOD("_on_crt_toggled"),
                         &ToolPanel::_on_crt_toggled);
    ClassDB::bind_method(D_METHOD("_on_dither_toggled"),
                         &ToolPanel::_on_dither_toggled);
    ClassDB::bind_method(D_METHOD("_on_pixel_toggled"),
                         &ToolPanel::_on_pixel_toggled);
    ClassDB::bind_method(D_METHOD("_on_vhs_toggled"),
                         &ToolPanel::_on_vhs_toggled);
    ClassDB::bind_method(D_METHOD("_on_bloom_toggled"),
                         &ToolPanel::_on_bloom_toggled);
}

ToolPanel::ToolPanel()
    : m_posterize_container(NodeBuilder<SliderContainer>::create()),
      m_dither_container(NodeBuilder<SliderContainer>::create()),
      m_outline_container(NodeBuilder<VBoxContainer>::create()),
      m_crt_container(NodeBuilder<VBoxContainer>::create()),
      m_pixel_container(NodeBuilder<HBoxContainer>::create()),
      m_vhs_container(NodeBuilder<VBoxContainer>::create()),
      m_bloom_container(NodeBuilder<VBoxContainer>::create())
{
}

ToolPanel::~ToolPanel() {}

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
    m_vhs.instantiate();
    m_bloom.instantiate();

    /// Get UI nodes
    // ApplyToContainer
    m_apply_option_btn =
        get_node<OptionButton>("ApplyToContainer/OptionButton");
    // ToggleContainer
    m_cel_toggle = get_node<CheckButton>("ToggleContainer/CelToggle");
    m_outline_toggle = get_node<CheckButton>("ToggleContainer/OutlineToggle");
    m_invert_toggle = get_node<CheckButton>("ToggleContainer/InvertToggle");
    m_crt_toggle = get_node<CheckButton>("ToggleContainer/CRTToggle");
    m_dither_toggle = get_node<CheckButton>("ToggleContainer/DitherToggle");
    m_pixel_toggle = get_node<CheckButton>("ToggleContainer/PixelToggle");
    m_vhs_toggle = get_node<CheckButton>("ToggleContainer/VHSToggle");
    m_bloom_toggle = get_node<CheckButton>("ToggleContainer/BloomToggle");

    // root
    m_effect_list = get_node<ItemList>("EffectList");
    m_tab_container = get_node<TabContainer>("TabContainer");

    /// Check if "gotten" UI nodes even exist
    ERR_FAIL_COND_MSG(!m_apply_option_btn,
                      "ERROR: Could not find OptionButton node!");
    ERR_FAIL_COND_MSG(!m_cel_toggle, "ERROR: Could not find CelToggle node!");
    ERR_FAIL_COND_MSG(!m_outline_toggle,
                      "ERROR: Could not find OutlineToggle node!");
    ERR_FAIL_COND_MSG(!m_invert_toggle,
                      "ERROR: Could not find InvertToggle node!");
    ERR_FAIL_COND_MSG(!m_crt_toggle, "ERROR: Could not find CRTToggle node!");
    ERR_FAIL_COND_MSG(!m_dither_toggle,
                      "ERROR: Could not find DitherToggle node!");
    ERR_FAIL_COND_MSG(!m_pixel_toggle,
                      "ERROR: Could not find PixelToggle node!");
    ERR_FAIL_COND_MSG(!m_vhs_toggle, "ERROR: Could not find VHSToggle node!");
    ERR_FAIL_COND_MSG(!m_bloom_toggle,
                      "ERROR: Could not find BloomToggle node!");
    ERR_FAIL_COND_MSG(!m_effect_list, "ERROR: Could not find EffectList node!");
    ERR_FAIL_COND_MSG(!m_tab_container,
                      "ERROR: Could not find TabContainer node!");

    /// NodeBuilder object initialization, per effect
    setup_cel();
    setup_outline();
    setup_crt();
    setup_dither();
    setup_pixel();
    setup_vhs();
    setup_bloom();

    /// Connect to signals
    m_cel_toggle->connect("toggled", Callable(this, "_on_cel_toggled"));
    m_outline_toggle->connect("toggled", Callable(this, "_on_outline_toggled"));
    m_invert_toggle->connect("toggled", Callable(this, "_on_invert_toggled"));
    m_crt_toggle->connect("toggled", Callable(this, "_on_crt_toggled"));
    m_dither_toggle->connect("toggled", Callable(this, "_on_dither_toggled"));
    m_pixel_toggle->connect("toggled", Callable(this, "_on_pixel_toggled"));
    m_vhs_toggle->connect("toggled", Callable(this, "_on_vhs_toggled"));
    m_bloom_toggle->connect("toggled", Callable(this, "_on_bloom_toggled"));
}

void ToolPanel::_process(double delta)
{
    if (Engine::get_singleton()->is_editor_hint())
    {
        if (m_edited_scene_root)
        {
            if (m_apply_option_btn)
            {
                m_apply_option_btn->clear();
            }

            m_camera3d = nullptr;
            m_world_environment = nullptr;
            m_camera3d_option_index = -1;
            m_world_environment_option_index = -1;

            // Traverse the current edited scene's children
            for (const auto &child : m_edited_scene_root->get_children())
            {
                Object *child_obj = child.get_validated_object();
                ERR_CONTINUE_MSG(!child_obj,
                                 "ERROR: Could not get valid object from node");

                // Checking to see if a Camera3D Object is a child in the tree
                // scene, as we need it
                if (Camera3D *c3d = Object::cast_to<Camera3D>(child_obj))
                {
                    m_camera3d = c3d;
                    UtilityFunctions::print(
                        "Found Camera3D node in edited scene!");
                }

                // Checking to see if a WorldEnvironment Object is a child in
                // the tree scene, as we need it
                if (WorldEnvironment *w_env =
                        Object::cast_to<WorldEnvironment>(child_obj))
                {
                    m_world_environment = w_env;
                    UtilityFunctions::print(
                        "Found WorldEnvironment node in edited scene!");
                }
            }

            if (m_apply_option_btn)
            {
                if (m_camera3d)
                {
                    Ref<Compositor> c3d_cmp = m_camera3d->get_compositor();
                    Ref<Compositor> wenv_cmp =
                        m_world_environment->get_compositor();
                    if (!c3d_cmp.is_valid())
                    {
                        c3d_cmp.instantiate();
                        m_camera3d->set_compositor(c3d_cmp);
                    }
                    if (!wenv_cmp.is_valid())
                    {
                        wenv_cmp.instantiate();
                        m_world_environment->set_compositor(wenv_cmp);
                    }

                    m_camera3d_compositor = c3d_cmp.ptr();
                    m_world_environment_compositor = wenv_cmp.ptr();

                    ERR_FAIL_COND_MSG(!m_camera3d_compositor,
                                      "ERROR: Camera3D compositor invalid!");
                    ERR_FAIL_COND_MSG(
                        !m_world_environment_compositor,
                        "ERROR: WorldEnvironment compositor invalid!");

                    m_camera3d_option_index =
                        m_apply_option_btn->get_item_count();
                    UtilityFunctions::print(
                        "Camera3D Option Index: " +
                        String::num(m_camera3d_option_index));
                    m_apply_option_btn->add_item("Camera3D",
                                                 m_camera3d_option_index);
                }
                if (m_world_environment)
                {
                    m_world_environment_option_index =
                        m_apply_option_btn->get_item_count();
                    UtilityFunctions::print(
                        "WEnv Option Index: " +
                        String::num(m_world_environment_option_index));
                    m_apply_option_btn->add_item(
                        "WorldEnvironment", m_world_environment_option_index);
                }
            }

            m_edited_scene_root = nullptr;
        }
        if (m_apply_option_btn)
        {
            int32_t selected_idx = m_apply_option_btn->get_selected();
            if (selected_idx == m_camera3d_option_index)
            {
                if (m_camera3d_compositor)
                {
                    m_camera3d_compositor->set_compositor_effects(
                        m_effect_arr->get_effects());
                    if (m_world_environment_compositor)
                        m_world_environment_compositor->set_compositor_effects(
                            {});
                }
            }
            else if (selected_idx == m_world_environment_option_index)
            {
                if (m_world_environment_compositor)
                {
                    m_world_environment_compositor->set_compositor_effects(
                        m_effect_arr->get_effects());
                    if (m_camera3d_compositor)
                        m_camera3d_compositor->set_compositor_effects({});
                }
            }
        }
    }
    if (m_outline.is_valid())
        m_outline->m_dt->set(delta);
    if (m_vhs.is_valid())
        m_vhs->m_dt->set(m_vhs->m_dt->get() + delta);
}

void ToolPanel::_on_cel_toggled(bool toggled_on)
{
    m_cel->set_enabled(toggled_on);

    if (toggled_on)
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
    if (toggled_on)
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
    if (toggled_on)
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

    if (toggled_on)
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

    if (toggled_on)
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

    if (toggled_on)
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

void ToolPanel::_on_vhs_toggled(bool toggled_on)
{
    m_vhs->set_enabled(toggled_on);

    if (toggled_on)
    {
        ADD_EFFECT(m_vhs);

        m_tab_container->add_child(m_vhs_container.get());

        UtilityFunctions::print("VHS effect toggled on");
    }
    else
    {
        REMOVE_EFFECT(m_vhs);

        m_tab_container->remove_child(m_vhs_container.get());

        UtilityFunctions::print("VHS effect toggled off");
    }
}

void ToolPanel::_on_bloom_toggled(bool toggled_on)
{
    m_bloom->set_enabled(toggled_on);

    if (toggled_on)
    {
        ADD_EFFECT(m_bloom);

        m_tab_container->add_child(m_bloom_container.get());

        UtilityFunctions::print("Bloom effect toggled on");
    }
    else
    {
        REMOVE_EFFECT(m_bloom);

        m_tab_container->remove_child(m_bloom_container.get());

        UtilityFunctions::print("Bloom effect toggled off");
    }
}

void ToolPanel::setup_cel()
{
    m_posterize_container.slider_container_init(
        "Posterize", "Levels", 1.0, 2.0, 32.0, m_cel->m_levels,
        encapsulated_callable(float, m_cel, m_levels));
}

void ToolPanel::setup_outline()
{
    m_outline_container.call(&VBoxContainer::set_name, "Outline");
    auto width_container =
        m_outline_container.add_child<SliderContainer>().slider_container_init(
            "Outline Width", "Outline Width", 0.001, 0.0, 0.01,
            m_outline->m_outline_width,
            encapsulated_callable(float, m_outline, m_outline_width));

    auto mul_container =
        m_outline_container.add_child<SliderContainer>().slider_container_init(
            "Outline Width Step", "Outline Width Step", 0.01, 0.01, 1.0,
            m_outline->m_outline_mul,
            encapsulated_callable(float, m_outline, m_outline_mul));

    m_outline_container.add_child<CheckBox>()
        .call(&CheckBox::set_text, "Jitter")
        .call(&CheckBox::connect, "toggled",
              encapsulated_callable(bool, m_outline, m_jitter_toggle), 0u);

    auto amp_container =
        m_outline_container.add_child<SliderContainer>().slider_container_init(
            "Jitter Amplitude", "Jitter Amplitude", 0.01, 0.01, 0.1,
            m_outline->m_jitter_amp,
            encapsulated_callable(float, m_outline, m_jitter_amp));
    auto freq_container =
        m_outline_container.add_child<SliderContainer>().slider_container_init(
            "Jitter Frequency", "Jitter Frequency", 0.01, 0.01, 0.1,
            m_outline->m_jitter_freq,
            encapsulated_callable(float, m_outline, m_jitter_freq));
    auto color_container = m_outline_container.add_child<HBoxContainer>();

    color_container.add_child<Label>().call(&Label::set_text,
                                            "Open Color Picker");
    color_container.add_child<ColorPickerButton>()
        .call(&ColorPickerButton::set_text, "Color Picker Button")
        .call(&ColorPickerButton::connect, "color_changed",
              callable_mp(m_outline.ptr(), &OutlineShader::set_outline_color),
              0u);
}

void ToolPanel::setup_crt()
{
    m_crt_container.call(&VBoxContainer::set_name, "CRT");
    auto curvature_container =
        m_crt_container.add_child<SliderContainer>().slider_container_init(
            "Curvature", "Curvature", 1.0, 0.0, 10.0, m_crt->m_curvature,
            encapsulated_callable(float, m_crt, m_curvature));
    auto vignette_mul_container =
        m_crt_container.add_child<SliderContainer>().slider_container_init(
            "Vignette Multiplier", "Vignette Multiplier", 1.0, 0.0, 10.0,
            m_crt->m_vignette_mul,
            encapsulated_callable(float, m_crt, m_vignette_mul));
    auto brightness_container =
        m_crt_container.add_child<SliderContainer>().slider_container_init(
            "Brightness", "Brightness", 0.1, 0.0, 10.0, m_crt->m_brightness,
            encapsulated_callable(float, m_crt, m_brightness));
}

void ToolPanel::setup_dither()
{
    m_dither_container.slider_container_init(
        "Dither", "Gamma Correction Amount", 0.1, 0.0, 10.0,
        m_dither->m_gamma_correction,
        encapsulated_callable(float, m_dither, m_gamma_correction));
}

void ToolPanel::setup_pixel()
{
    m_pixel_container.call(&HBoxContainer::set_name, "Pixelize");
    auto label = m_pixel_container.add_child<Label>().call(
        &Label::set_text, "Target Width and Height");

    auto width_spin_box =
        m_pixel_container.add_child<SpinBox>()
            .call(&SpinBox::set_step, 1.0)
            .call(&SpinBox::set_min, 1.0)
            .call(&SpinBox::set_max,
                  static_cast<double>(
                      DisplayServer::get_singleton()->screen_get_size().x))
            .call(&SpinBox::set_value,
                  static_cast<double>(m_pixel->target_width->get()))
            .call(&SpinBox::connect, "value_changed",
                  encapsulated_callable(int, m_pixel, target_width), 0u);

    auto height_spin_box =
        m_pixel_container.add_child<SpinBox>()
            .call(&SpinBox::set_step, 1.0)
            .call(&SpinBox::set_min, 1.0)
            .call(&SpinBox::set_max,
                  static_cast<double>(
                      DisplayServer::get_singleton()->screen_get_size().y))
            .call(&SpinBox::set_value,
                  static_cast<double>(m_pixel->target_height->get()))
            .call(&SpinBox::connect, "value_changed",
                  encapsulated_callable(int, m_pixel, target_height), 0u);
}

// TODO: refactor this function and get rid of the static raw pointers, 200
// lines just for this currently lol
void ToolPanel::setup_vhs()
{
    m_vhs_container.call(&VBoxContainer::set_name, "VHS");
    auto scanline_checkbox =
        m_vhs_container.add_child<CheckBox>()
            .call(&CheckBox::set_text, "Enable Scanlines")
            .call(&CheckBox::connect, "toggled",
                  create_custom_callable_lambda(
                      this,
                      [&](bool toggled_on)
                      {
                          m_vhs->m_scanline_enabled->set(toggled_on);

                          static SliderContainer *raw_blend = nullptr;
                          static SliderContainer *raw_height = nullptr;
                          static SliderContainer *raw_intensity = nullptr;
                          static SliderContainer *raw_scroll = nullptr;

                          if (toggled_on)
                          {
                              auto scanline_blend_container =
                                  m_vhs_container.add_child<SliderContainer>()
                                      .slider_container_init(
                                          "Scanline Blend Factor",
                                          "Scanline Blend Factor", 0.01, 0.0,
                                          1.0, m_vhs->m_scanline_blend_factor,
                                          encapsulated_callable(
                                              float, m_vhs,
                                              m_scanline_blend_factor));

                              raw_blend = scanline_blend_container.get();

                              auto scanline_height_container =
                                  m_vhs_container.add_child<SliderContainer>()
                                      .slider_container_init(
                                          "Scanline Height", "Scanline Height",
                                          1.0, 1.0, 20.0,
                                          m_vhs->m_scanline_height,
                                          encapsulated_callable(
                                              float, m_vhs, m_scanline_height));

                              raw_height = scanline_height_container.get();

                              auto scanline_intensity_container =
                                  m_vhs_container.add_child<SliderContainer>()
                                      .slider_container_init(
                                          "Scanline Intensity",
                                          "Scanline Intensity", 0.01, 0.0, 1.0,
                                          m_vhs->m_scanline_intensity,
                                          encapsulated_callable(
                                              float, m_vhs,
                                              m_scanline_intensity));

                              raw_intensity =
                                  scanline_intensity_container.get();

                              auto scanline_scroll_container =
                                  m_vhs_container.add_child<SliderContainer>()
                                      .slider_container_init(
                                          "Scanline Scroll Speed",
                                          "Scanline Scroll Speed", 1.0, 0.0,
                                          100.0, m_vhs->m_scanline_scroll_speed,
                                          encapsulated_callable(
                                              float, m_vhs,
                                              m_scanline_scroll_speed));

                              raw_scroll = scanline_scroll_container.get();
                          }
                          else
                          {
                              m_vhs_container.try_remove_child(raw_blend);
                              m_vhs_container.try_remove_child(raw_height);
                              m_vhs_container.try_remove_child(raw_intensity);
                              m_vhs_container.try_remove_child(raw_scroll);
                          }
                      }),
                  0u);

    auto grain_checkbox =
        m_vhs_container.add_child<CheckBox>()
            .call(&CheckBox::set_text, "Enable Grain")
            .call(&CheckBox::connect, "toggled",
                  create_custom_callable_lambda(
                      this,
                      [&](bool toggled_on)
                      {
                          m_vhs->m_grain_enabled->set(toggled_on);

                          static SliderContainer *raw_grain = nullptr;

                          if (toggled_on)
                          {
                              auto grain_intensity_container =
                                  m_vhs_container.add_child<SliderContainer>()
                                      .slider_container_init(
                                          "Grain Intensity", "Grain Intensity",
                                          0.1, 0.0, 10.0,
                                          m_vhs->m_grain_intensity,
                                          encapsulated_callable(
                                              float, m_vhs, m_grain_intensity));
                              raw_grain = grain_intensity_container.get();
                          }
                          else
                          {
                              m_vhs_container.try_remove_child(raw_grain);
                          }
                      }),
                  0u);

    auto vertical_band_checkbox =
        m_vhs_container.add_child<CheckBox>()
            .call(&CheckBox::set_text, "Enable Vertical Bands")
            .call(&CheckBox::connect, "toggled",
                  create_custom_callable_lambda(
                      this,
                      [&](bool toggled_on)
                      {
                          m_vhs->m_vertical_band_enabled->set(toggled_on);

                          static SliderContainer *raw_speed = nullptr;
                          static SliderContainer *raw_height = nullptr;
                          static SliderContainer *raw_intensity = nullptr;
                          static SliderContainer *raw_choppiness = nullptr;
                          static SliderContainer *raw_static = nullptr;
                          static SliderContainer *raw_warp = nullptr;

                          if (toggled_on)
                          {
                              auto vertical_band_speed_container =
                                  m_vhs_container.add_child<SliderContainer>()
                                      .slider_container_init(
                                          "Vertical Band Speed",
                                          "Vertical Band Speed", 0.01, 0.0, 5.0,
                                          m_vhs->m_vertical_band_speed,
                                          encapsulated_callable(
                                              float, m_vhs,
                                              m_vertical_band_speed));

                              raw_speed = vertical_band_speed_container.get();

                              auto vertical_band_height_container =
                                  m_vhs_container.add_child<SliderContainer>()
                                      .slider_container_init(
                                          "Vertical Band Height",
                                          "Vertical Band Height", 0.001, 0.001,
                                          0.1, m_vhs->m_vertical_band_height,
                                          encapsulated_callable(
                                              float, m_vhs,
                                              m_vertical_band_height));

                              raw_height = vertical_band_height_container.get();

                              auto vertical_band_intensity_container =
                                  m_vhs_container.add_child<SliderContainer>()
                                      .slider_container_init(
                                          "Vertical Band Intensity",
                                          "Vertical Band Intensity", 0.01, 0.0,
                                          1.0, m_vhs->m_vertical_band_intensity,
                                          encapsulated_callable(
                                              float, m_vhs,
                                              m_vertical_band_intensity));

                              raw_intensity =
                                  vertical_band_intensity_container.get();

                              auto vertical_band_choppiness_container =
                                  m_vhs_container.add_child<SliderContainer>()
                                      .slider_container_init(
                                          "Vertical Band Choppiness",
                                          "Vertical Band Choppiness", 0.01, 0.0,
                                          1.0,

                                          m_vhs->m_vertical_band_choppiness,
                                          encapsulated_callable(
                                              float, m_vhs,
                                              m_vertical_band_choppiness));

                              raw_choppiness =
                                  vertical_band_choppiness_container.get();

                              auto vertical_band_static_container =
                                  m_vhs_container.add_child<SliderContainer>()
                                      .slider_container_init(
                                          "Vertical Band Static Amount",
                                          "Vertical Band Static Amount", 0.001,
                                          0.0, 0.1,
                                          m_vhs->m_vertical_band_static_amount,
                                          encapsulated_callable(
                                              float, m_vhs,
                                              m_vertical_band_static_amount));

                              raw_static = vertical_band_static_container.get();

                              auto vertical_band_warp_container =
                                  m_vhs_container.add_child<SliderContainer>()
                                      .slider_container_init(
                                          "Vertical Band Warp Factor",
                                          "Vertical Band Warp Factor", 0.001,
                                          0.0, 0.1,
                                          m_vhs->m_vertical_band_warp_factor,
                                          encapsulated_callable(
                                              float, m_vhs,
                                              m_vertical_band_warp_factor));

                              raw_warp = vertical_band_warp_container.get();
                          }
                          else
                          {
                              m_vhs_container.try_remove_child(raw_speed);
                              m_vhs_container.try_remove_child(raw_height);
                              m_vhs_container.try_remove_child(raw_intensity);
                              m_vhs_container.try_remove_child(raw_choppiness);
                              m_vhs_container.try_remove_child(raw_static);
                              m_vhs_container.try_remove_child(raw_warp);
                          }
                      }),
                  0u);
}

void ToolPanel::setup_bloom()
{
    m_bloom_container.call(&VBoxContainer::set_name, "Bloom");

    m_bloom_container.add_child<SliderContainer>().slider_container_init(
        "Threshold", "Threshold", 0.01, 0.0, 10.0, m_bloom->m_threshold,
        encapsulated_callable(float, m_bloom, m_threshold));
    m_bloom_container.add_child<SliderContainer>().slider_container_init(
        "Radius", "Radius", 0.001, 0.0, 10.0, m_bloom->m_radius,
        encapsulated_callable(float, m_bloom, m_radius));

    m_bloom_container.add_child<SliderContainer>().slider_container_init(
        "Strength", "Strength", 0.001, 0.0, 1.0, m_bloom->m_strength,
        encapsulated_callable(float, m_bloom, m_strength));
}

void ToolPanel::set_edited_scene_root(Node *edited_scene_root)
{
    m_edited_scene_root = edited_scene_root;
}