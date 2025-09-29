#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "base_shader.hpp"

using namespace godot;

class CRTShader : public BaseShader 
{
    GDCLASS(CRTShader, BaseShader);

private:
    void init_compute(const String &shader_filename) override;

    float m_curvature = 7.0f;
    float m_vignette_mul = 2.0f;
    float m_brightness = 0.9f;
protected:
    static void _bind_methods();
public:
    CRTShader();
    ~CRTShader();

    void _notification(int what) override;
    void _render_callback(int32_t, RenderData *) override;

    void set_curvature(float curvature);
    float get_curvature() const;
    void set_vignette_mul(float vignette_mul);
    float get_vignette_mul() const;
    void set_brightness(float brightness);
    float get_brightness() const;
};