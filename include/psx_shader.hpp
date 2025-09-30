#pragma once

#include "base_shader.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

class PSXShader : public BaseShader
{
    GDCLASS(PSXShader, BaseShader);

private:
    void init_compute(const String &shader_filename) override;

    float m_dither_amount = 0.5f;
protected:
    static void _bind_methods();
public:
    PSXShader();
    ~PSXShader();

    void _notification(int what) override;
    void _render_callback(int32_t, RenderData *) override;

    void set_dither_amount(float dither_amount);
    float get_dither_amount() const;
};