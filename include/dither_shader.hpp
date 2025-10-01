#pragma once

#include "base_shader.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class DitherShader : public BaseShader
{
    GDCLASS(DitherShader, BaseShader);

private:
    void init_compute(const String &shader_filename) override;

    float m_gamma_correction = 2.2f;
protected:
    static void _bind_methods();
public:
    DitherShader();
    ~DitherShader();

    void _notification(int what) override;
    void _render_callback(int32_t, RenderData *) override;

    void set_gamma_correction(float gamma_correction);
    float get_gamma_correction() const;
};