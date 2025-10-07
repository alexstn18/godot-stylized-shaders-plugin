#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "base_shader.hpp"
#include "godot_cpp/classes/wrapped.hpp"

using namespace godot;

class VHSShader : public BaseShader
{
    GDCLASS(VHSShader, BaseShader);

private:
    float m_scanline_blend_factor       = .1f;
    float m_scanline_height             = 4.f;
    float m_scanline_intensity          = .25f;
    float m_scanline_scroll_speed       = 16.f;
    bool  m_scanline_enabled            = false;
    float m_grain_intensity             = 2.f;
    bool  m_grain_enabled               = false;
    float m_vertical_band_speed         = .2f;
    float m_vertical_band_height        = .01f;
    float m_vertical_band_intensity     = .2f;
    float m_vertical_band_choppiness    = .2f;
    float m_vertical_band_static_amount = .02f;
    float m_vertical_band_warp_factor   = .005f;
    bool  m_vertical_band_enabled       = false;
    
    float m_dt = 0.f;

    void init_compute(const String &shader_filename) override;
protected:
    static void _bind_methods();
public:
    VHSShader();
    ~VHSShader();

    void _notification(int what) override;
    void _render_callback(int32_t, RenderData *) override;

    float get_scanline_blend_factor() const;
    void  set_scanline_blend_factor(float factor);
    float get_scanline_height() const;
    void  set_scanline_height(float height);
    float get_scanline_intensity() const;
    void  set_scanline_intensity(float intensity);
    float get_scanline_scroll_speed() const;
    void  set_scanline_scroll_speed(float speed);
    bool  get_scanline_enabled() const;
    void  set_scanline_enabled(bool enabled);
    float get_grain_intensity() const;
    void  set_grain_intensity(float intensity);
    bool  get_grain_enabled() const;
    void  set_grain_enabled(bool enabled);
    float get_vertical_band_speed() const;
    void  set_vertical_band_speed(float speed);
    float get_vertical_band_height() const;
    void  set_vertical_band_height(float height);
    float get_vertical_band_intensity() const;
    void  set_vertical_band_intensity(float intensity);
    float get_vertical_band_choppiness() const;
    void  set_vertical_band_choppiness(float choppiness);
    float get_vertical_band_static_amount() const;
    void  set_vertical_band_static_amount(float amount);
    float get_vertical_band_warp_factor() const;
    void  set_vertical_band_warp_factor(float factor);
    bool  get_vertical_band_enabled() const;
    void  set_vertical_band_enabled(bool enabled);
    void  set_dt(double dt);
    float get_dt() const;
};