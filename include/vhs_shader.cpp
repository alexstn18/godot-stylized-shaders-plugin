#include "vhs_shader.hpp"
#include <godot_cpp/classes/compositor_effect.hpp>
#include <godot_cpp/classes/render_scene_buffers_rd.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>

void VHSShader::_bind_methods()
{
    
}

VHSShader::VHSShader()
{
    construct();
    queue_callable_on_render_thread(callable_mp(this, &VHSShader::init_compute).bind("vhs.glsl"));
}

VHSShader::~VHSShader() {}

void VHSShader::init_compute(const String &shader_filename) { BaseShader::init_compute(shader_filename); }

void VHSShader::_notification(int what)
{
    UtilityFunctions::print("PostProcessShader notification: " + String::num(what));
    
    if (what == NOTIFICATION_PREDELETE && m_device)
    {
        free_shader();
    }
}

void VHSShader::_render_callback(int32_t p_effect_callback_type, RenderData *p_render_data)
{
    Ref<RenderSceneBuffersRD> buffers; buffers.instantiate();
    Vector2i size = get_buffers_internal_size(p_render_data, buffers);
    ERR_FAIL_COND_MSG(size.x == 0 || size.y == 0, "Buffer size is 0");
    
    PackedFloat32Array push_constant = {(float)size.x, (float)size.y,
                                        m_scanline_blend_factor,
                                        m_scanline_height,
                                        m_scanline_intensity,
                                        m_scanline_scroll_speed,
                                        (float)m_scanline_enabled,
                                        m_grain_intensity,
                                        (float)m_grain_enabled,
                                        m_vertical_band_speed,
                                        m_vertical_band_height,
                                        m_vertical_band_intensity,
                                        m_vertical_band_choppiness,
                                        m_vertical_band_static_amount,
                                        m_vertical_band_warp_factor,
                                        (float)m_vertical_band_enabled,
                                        m_dt,
                                        0.0f, 0.0f, 0.0f};
    ERR_FAIL_COND_MSG(push_constant.is_empty(), "Push constant is empty/invalid!");
    
    base_compute_update(p_effect_callback_type, p_render_data, buffers, push_constant, size);
}

float VHSShader::get_scanline_blend_factor() const { return m_scanline_blend_factor; }
void  VHSShader::set_scanline_blend_factor(float factor) { m_scanline_blend_factor = factor; }
float VHSShader::get_scanline_height() const { return m_scanline_height; }
void  VHSShader::set_scanline_height(float height) { m_scanline_height = height; }
float VHSShader::get_scanline_intensity() const { return m_scanline_intensity; }
void  VHSShader::set_scanline_intensity(float intensity) { m_scanline_intensity = intensity; }
float VHSShader::get_scanline_scroll_speed() const { return m_scanline_scroll_speed; }
void  VHSShader::set_scanline_scroll_speed(float speed) { m_scanline_scroll_speed = speed; }
bool  VHSShader::get_scanline_enabled() const { return m_scanline_enabled; }
void  VHSShader::set_scanline_enabled(bool enabled) { m_scanline_enabled = enabled; }
float VHSShader::get_grain_intensity() const { return m_grain_intensity; }
void  VHSShader::set_grain_intensity(float intensity) { m_grain_intensity = intensity; }
bool  VHSShader::get_grain_enabled() const { return m_grain_enabled; }
void  VHSShader::set_grain_enabled(bool enabled) { m_grain_enabled = enabled; }
float VHSShader::get_vertical_band_speed() const { return m_vertical_band_speed; }
void  VHSShader::set_vertical_band_speed(float speed) { m_vertical_band_speed = speed; }
float VHSShader::get_vertical_band_height() const { return m_vertical_band_height; }
void  VHSShader::set_vertical_band_height(float height) { m_vertical_band_height = height; }
float VHSShader::get_vertical_band_intensity() const { return m_vertical_band_intensity; }
void  VHSShader::set_vertical_band_intensity(float intensity) { m_vertical_band_intensity = intensity; }
float VHSShader::get_vertical_band_choppiness() const { return m_vertical_band_choppiness; }
void  VHSShader::set_vertical_band_choppiness(float choppiness) { m_vertical_band_choppiness = choppiness; }
float VHSShader::get_vertical_band_static_amount() const { return m_vertical_band_static_amount; }
void  VHSShader::set_vertical_band_static_amount(float amount) { m_vertical_band_static_amount = amount; }
float VHSShader::get_vertical_band_warp_factor() const { return m_vertical_band_warp_factor; }
void  VHSShader::set_vertical_band_warp_factor(float factor) { m_vertical_band_warp_factor = factor; }
bool  VHSShader::get_vertical_band_enabled() const { return m_vertical_band_enabled; }
void  VHSShader::set_vertical_band_enabled(bool enabled) { m_vertical_band_enabled = enabled; }
float VHSShader::get_dt() const { return m_dt; }
void  VHSShader::set_dt(double dt) { m_dt = (float)dt; }