#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "base_shader.hpp"

using namespace godot;

class CelShader : public BaseShader 
{
    GDCLASS(CelShader, BaseShader);

private:
    int m_levels = 16;

    void init_compute(const String &shader_filename) override;
protected:
    static void _bind_methods();
public:
    CelShader();
    ~CelShader();

    void _notification(int what) override;
    void _render_callback(int32_t, RenderData *) override;
    void set_levels(int levels);
    int get_levels() const;
};