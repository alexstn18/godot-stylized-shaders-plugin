#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/compositor_effect.hpp>
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

using namespace godot;

class OutlineShader : public CompositorEffect
{
    GDCLASS(OutlineShader, CompositorEffect);

private:
    void init_compute();

	String m_shader_template = "";
    RenderingDevice *m_device = nullptr;
    RID m_shader;
    RID m_pipeline;
    RID m_depth_sampler;
protected:
    static void _bind_methods();
public:
    OutlineShader();
    ~OutlineShader();

    void _render_callback(int32_t, RenderData *) override;
    void _notification(int what);
};