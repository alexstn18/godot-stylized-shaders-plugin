#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "base_shader.hpp"

using namespace godot;

class PixelShader : public BaseShader
{
    GDCLASS(PixelShader, BaseShader);

private:
    void init_compute(const String &shader_filename) override;

    int m_target_width = 320;
    int m_target_height = 180;
protected:
    static void _bind_methods();
public:
    PixelShader();
    ~PixelShader() = default;

    void _notification(int what) override;
    void _render_callback(int32_t, RenderData *) override;
    
    void set_target_width(int target_width);
    int get_target_width() const;
    void set_target_height(int target_height);
    int get_target_height() const;
};