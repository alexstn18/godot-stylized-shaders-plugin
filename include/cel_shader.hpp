#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "base_shader.hpp"

using namespace godot;

class CelShader : public BaseShader 
{
    GDCLASS(CelShader, BaseShader);

private:
    void init_compute(const String &shader_filename) override;
protected:
    static void _bind_methods();
public:
    CelShader();
    ~CelShader();
    EncapsuledData<float> *m_levels = memnew(EncapsuledData<float>(16.f));

    void _notification(int what) override;
    void _render_callback(int32_t, RenderData *) override;
};