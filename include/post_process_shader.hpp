#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/compositor_effect.hpp>
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/mutex.hpp>

using namespace godot;

class PostProcessShader : public CompositorEffect 
{
    GDCLASS(PostProcessShader, CompositorEffect);
    
private:
    void setup();
	String m_shader_template;

    String m_shader_code;
    RenderingDevice* m_device = nullptr;
    RID m_shader;
    RID m_pipeline;
    
    Ref<Mutex> m_mutex;

    bool m_shader_dirty = false;
protected:
    static void _bind_methods();
public:
    PostProcessShader();
    ~PostProcessShader();

    void _render_callback(int32_t p_effect_callback_type, RenderData *p_render_data) override;
    void _notification(int what);

    void set_shader_code(const String& value);
    String get_shader_code() const;

    bool _check_shader();
};