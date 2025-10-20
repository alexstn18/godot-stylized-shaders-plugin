#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/check_button.hpp>
#include <godot_cpp/classes/item_list.hpp>
#include <godot_cpp/classes/tab_container.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/world_environment.hpp>
#include <godot_cpp/classes/compositor.hpp>
#include <godot_cpp/classes/compositor_effect.hpp>
#include <godot_cpp/classes/editor_property.hpp>

#include "effect_array.hpp"
#include "invert_shader.hpp"
#include "manual_array_inspector.hpp"
#include "outline_shader.hpp"
#include "cel_shader.hpp"
#include "crt_shader.hpp"
#include "dither_shader.hpp"
#include "pixel_shader.hpp"
#include "vhs_shader.hpp"
#include "bloom_shader.hpp"

#include "slider_container.hpp"
#include "util/node_builder.hpp"

using namespace godot;

// Convention:
// Please name the Control derived vars similar to their names as nodes
// e.g. m_cel_toggle == "CelToggle"

class ToolPanel : public VBoxContainer 
{
    GDCLASS(ToolPanel, VBoxContainer);

private:
    // UI (gotten from the UI scene)
    OptionButton *m_apply_option_btn = nullptr;
    CheckButton  *m_cel_toggle       = nullptr;
    CheckButton  *m_outline_toggle   = nullptr;
    CheckButton  *m_invert_toggle    = nullptr;
    CheckButton  *m_crt_toggle       = nullptr;
    CheckButton  *m_dither_toggle    = nullptr;
    CheckButton  *m_pixel_toggle     = nullptr;
    CheckButton  *m_vhs_toggle       = nullptr;
    CheckButton  *m_bloom_toggle     = nullptr;
    
    ManualArrayInspector *m_array_inspector = nullptr;
    TabContainer *m_tab_container    = nullptr;

    // UI (code/manually-made, mostly containers)
    NodeBuilder<SliderContainer> m_posterize_container;
    NodeBuilder<SliderContainer> m_dither_container;
    NodeBuilder<VBoxContainer>   m_outline_container;
    NodeBuilder<VBoxContainer>   m_crt_container;
    NodeBuilder<VBoxContainer>   m_vhs_container;
    NodeBuilder<VBoxContainer>   m_bloom_container;
    NodeBuilder<HBoxContainer>   m_pixel_container;

    // Compositor-related
    Compositor *m_camera3d_compositor = nullptr;
    Compositor *m_world_environment_compositor = nullptr;
    Ref<InvertShader> m_invert;
    Ref<OutlineShader> m_outline;
    Ref<CelShader> m_cel;
    Ref<CRTShader> m_crt;
    Ref<DitherShader> m_dither;
    Ref<PixelShader> m_pixel;
    Ref<VHSShader> m_vhs;
    Ref<BloomShader> m_bloom;
    Ref<EffectArray> m_effect_arr;

    // Other
    Node *m_edited_scene_root = nullptr;
    Camera3D *m_camera3d = nullptr;
    WorldEnvironment *m_world_environment = nullptr;
    int32_t m_camera3d_option_index = 0;
    int32_t m_world_environment_option_index = 0;

    void setup_cel();
    void setup_outline();
    void setup_invert();
    void setup_crt();
    void setup_dither();
    void setup_pixel();
    void setup_vhs();
    void setup_bloom();
protected:
    static void _bind_methods();
public:
    ToolPanel();
    ~ToolPanel();

    void _ready() override;
    void _process(double delta) override;
    void _on_cel_toggled(bool toggled_on);
    void _on_outline_toggled(bool toggled_on);
    void _on_invert_toggled(bool toggled_on);
    void _on_crt_toggled(bool toggled_on);
    void _on_dither_toggled(bool toggled_on);
    void _on_pixel_toggled(bool toggled_on);
    void _on_vhs_toggled(bool toggled_on);
    void _on_bloom_toggled(bool toggled_on);
    void set_edited_scene_root(Node *edited_scene_root);
};