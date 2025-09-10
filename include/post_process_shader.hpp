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
	String m_shader_template = String(
R"DELIM(
#version 450

// Invocations in the (x, y, z) dimension
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba16f, set = 0, binding = 0) uniform image2D color_image;

// Our push constant
layout(push_constant, std430) uniform Params {
	vec2 raster_size;
	vec2 reserved;
} params;

// The code we want to execute in each invocation
void main() {
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = ivec2(params.raster_size);

	if (uv.x >= size.x || uv.y >= size.y) {
		return;
	}

	vec4 color = imageLoad(color_image, uv);

	#COMPUTE_CODE

	imageStore(color_image, uv, color);
}
)DELIM");

    String m_shader_code = "";
    RenderingDevice* m_device = nullptr;
    RID m_shader;
    RID m_pipeline;
    
    Mutex m_mutex;

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