#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/compositor_effect.hpp>
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

using namespace godot;

class BaseShader : public CompositorEffect 
{
    GDCLASS(BaseShader, CompositorEffect);

private:
    String m_addon_path = "res://addons/GodotStylizedShadersPlugin/shaders/";
protected:
    RID m_shader;
    RID m_pipeline;
    RenderingDevice *m_device = nullptr;

    void free_shader();
    void construct();
    void queue_callable_on_render_thread(const Callable &c);
    static void _bind_methods();
    virtual void init_compute(const String &shader_filename);
public:
    virtual void _notification(int what) = 0;
};
