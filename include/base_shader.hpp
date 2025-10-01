#pragma once

#include "godot_cpp/classes/render_scene_buffers_rd.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/compositor_effect.hpp>
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <vector>
#include <functional>

using namespace godot;

class BaseShader : public CompositorEffect 
{
    GDCLASS(BaseShader, CompositorEffect);

private:
    String m_addon_path = "res://addons/GodotStylizedShadersPlugin/shaders/";
    TypedArray<Callable> m_uniform_callables;
    protected:
    RID m_shader;
    RID m_pipeline;
    RenderingDevice *m_device = nullptr;

    void free_shader();
    void construct();
    void queue_callable_on_render_thread(const Callable &c);
    void base_compute_update(int32_t p_effect_callback_type, RenderData *p_render_data, Ref<RenderSceneBuffersRD> &buffers, const PackedFloat32Array &push_constant, const Vector2i &size);

    static void _bind_methods();
    virtual void init_compute(const String &shader_filename);
    void push_back_callable(const Callable &c);
    Vector2i get_buffers_internal_size(RenderData *, Ref<RenderSceneBuffersRD> &) const;
public:
    virtual void _notification(int what) = 0;
};
