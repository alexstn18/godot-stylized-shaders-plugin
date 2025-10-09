#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "base_shader.hpp"

using namespace godot;

class BloomShader : public BaseShader
{
    GDCLASS(BloomShader, BaseShader);

private:
    RID m_downsample_shader;
    RID m_upsample_shader;
    RID m_add_shader;
    RID m_downsample_pipeline;
    RID m_upsample_pipeline;
    RID m_add_pipeline;
    RID m_bilinear_sampler;

    float m_threshold = 1.0f;
    float m_radius = 1.0f;

    void init_compute();
    void create_shader(const String &shader_path, RID &shader, RID &pipeline);
    void free_rid(RID &rid);
    Ref<RDUniform> get_sampler_uniform(const RID &image, int32_t binding = 0);
    Ref<RDUniform> get_image_uniform(const RID &image, int32_t binding = 0);
    Ref<RDUniform> get_buffer_uniform(const RID &buffer, int binding = 0);
protected:
    static void _bind_methods();
public:
    BloomShader();
    ~BloomShader();

    void _notification(int what) override;
    void _render_callback(int32_t, RenderData *) override;

    void set_threshold(float threshold) { m_threshold = threshold; }
    float get_threshold() const { return m_threshold; }
    void set_radius(float radius) { m_radius = radius; }
    float get_radius() const { return m_radius; }
};