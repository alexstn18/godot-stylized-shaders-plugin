#include "outline_shader.hpp"
#include "base_shader.hpp"
#include <godot_cpp/classes/compositor_effect.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rd_shader_source.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/render_scene_buffers_rd.hpp>
#include <godot_cpp/classes/render_scene_data_rd.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/rd_shader_file.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/render_scene_data_rd.hpp>
#include <godot_cpp/classes/rd_sampler_state.hpp>
#include <godot_cpp/variant/projection.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/engine.hpp>

void OutlineShader::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_outline_color", "color"), &OutlineShader::set_outline_color);
    ClassDB::bind_method(D_METHOD("get_outline_color"), &OutlineShader::get_outline_color);
    ClassDB::bind_method(D_METHOD("set_outline_width", "width"), &OutlineShader::set_outline_width);
    ClassDB::bind_method(D_METHOD("get_outline_width"), &OutlineShader::get_outline_width);
    ClassDB::bind_method(D_METHOD("set_outline_mul", "mul"), &OutlineShader::set_outline_mul);
    ClassDB::bind_method(D_METHOD("get_outline_mul"), &OutlineShader::get_outline_mul);
    ClassDB::bind_method(D_METHOD("set_jitter", "jitter"), &OutlineShader::set_jitter);
    ClassDB::bind_method(D_METHOD("get_jitter"), &OutlineShader::get_jitter);
    ClassDB::bind_method(D_METHOD("set_jitter_amp", "amp"), &OutlineShader::set_jitter_amp);
    ClassDB::bind_method(D_METHOD("get_jitter_amp"), &OutlineShader::get_jitter_amp);
    ClassDB::bind_method(D_METHOD("set_jitter_freq", "freq"), &OutlineShader::set_jitter_freq);
    ClassDB::bind_method(D_METHOD("get_jitter_freq"), &OutlineShader::get_jitter_freq);
}

OutlineShader::OutlineShader()
{
    construct();
    m_depth_sampler = RID();
    queue_callable_on_render_thread(callable_mp(this, &OutlineShader::init_compute).bind("outline.glsl"));
}

OutlineShader::~OutlineShader() {}

void OutlineShader::_notification(int what)
{
    UtilityFunctions::print("OutlineShader notification: " + String::num(what));
    
    if (what == NOTIFICATION_PREDELETE && m_device)
    {
        free_shader();
        
        if (m_depth_sampler.is_valid()) 
        {
            m_device->free_rid(m_depth_sampler);
            m_depth_sampler = RID();
            UtilityFunctions::print("Freed depth sampler");
        }
    }
}

void OutlineShader::_render_callback(int32_t p_effect_callback_type,
                                         RenderData *p_render_data)
{
    if(m_device && 
        p_effect_callback_type == EFFECT_CALLBACK_TYPE_POST_TRANSPARENT)
    {
        // Check if shader and pipeline are valid before proceeding
        ERR_FAIL_COND_MSG(!m_shader.is_valid(), "Shader is invalid in render callback!");
        ERR_FAIL_COND_MSG(!m_pipeline.is_valid(), "Pipeline is invalid in render callback!");
        
        Ref<RenderSceneBuffersRD> buffers = p_render_data->get_render_scene_buffers();
        RenderSceneData *scene_data = p_render_data->get_render_scene_data();
        if(buffers.is_valid() || !scene_data)
        {
            Vector2i size = buffers->get_internal_size();
            ERR_FAIL_COND_MSG(size.x == 0 || size.y == 0, "size is 0");

            const int x_groups = (size.x + 15) / 16;
            const int y_groups = (size.y + 15) / 16;

            auto inv_proj_mat = p_render_data->get_render_scene_data()->get_cam_projection().inverse();
            PackedFloat32Array push_constant = {m_outline_color.r,
                                                m_outline_color.g,
                                                m_outline_color.b,
                                                m_jitter_amp,
                                                (float)size.x,
                                                (float)size.y,
                                                inv_proj_mat[2].w,
                                                inv_proj_mat[3].w,
                                                m_outline_width,
                                                m_outline_mul,
                                                m_dt,
                                                (float)UtilityFunctions::randf(),
                                                (float)m_jitter_toggle,
                                                m_jitter_freq, 
                                                0.0f, 0.0f};
            ERR_FAIL_COND_MSG(push_constant.is_empty(), "push constant is empty");

            uint32_t view_count = buffers->get_view_count();
            
            for(int32_t i = 0; i < view_count; i++)
            {
                RID input_image = buffers->get_color_layer(i);
                ERR_CONTINUE_MSG(!input_image.is_valid(), "Invalid input image for view " + String::num(i));
                RID depth_texture = buffers->get_depth_layer(i);
                ERR_CONTINUE_MSG(!depth_texture.is_valid(), "Invalid depth texture for view " + String::num(i));

                Ref<RDUniform> uniform; uniform.instantiate();
                uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_IMAGE);
                uniform->set_binding(0);
                uniform->add_id(input_image);
                
                RID image_uniform_set = UniformSetCacheRD::get_cache(m_shader, 0, {uniform});
                ERR_CONTINUE_MSG(!image_uniform_set.is_valid(), "Failed to create color image uniform set for view " + String::num(i));

                uniform.instantiate();
                uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE);
                uniform->set_binding(0);
                uniform->add_id(m_depth_sampler);
                uniform->add_id(depth_texture);

                RID depth_uniform_set = UniformSetCacheRD::get_cache(m_shader, 1, {uniform});
                ERR_CONTINUE_MSG(!image_uniform_set.is_valid(), "Failed to create depth uniform set for view " + String::num(i));

                auto compute_list = m_device->compute_list_begin();
                m_device->compute_list_bind_compute_pipeline(compute_list, m_pipeline);
                m_device->compute_list_bind_uniform_set(compute_list, image_uniform_set, 0);
                m_device->compute_list_bind_uniform_set(compute_list, depth_uniform_set, 1);
                m_device->compute_list_set_push_constant(compute_list, push_constant.to_byte_array(), push_constant.size() * 4);
                m_device->compute_list_dispatch(compute_list, x_groups, y_groups, 1);
                m_device->compute_list_end();
            }
        }
    }
}

void OutlineShader::init_compute(const String &shader_filename)
{
    BaseShader::init_compute(shader_filename);

    Ref<RDSamplerState> state;
    state.instantiate();
    state->set_min_filter(RenderingDevice::SAMPLER_FILTER_LINEAR);
    state->set_mag_filter(RenderingDevice::SAMPLER_FILTER_LINEAR);
    m_depth_sampler = m_device->sampler_create(state);
    ERR_FAIL_COND_MSG(!m_depth_sampler.is_valid(), "Failed to create sampler!");
}

void OutlineShader::set_outline_color(Color color) { m_outline_color = color; }
Color OutlineShader::get_outline_color() const { return m_outline_color; }
void OutlineShader::set_outline_width(double width) { m_outline_width = static_cast<float>(width); }
float OutlineShader::get_outline_width() const { return m_outline_width; }
void OutlineShader::set_outline_mul(double mul) { m_outline_mul = static_cast<float>(mul); }
float OutlineShader::get_outline_mul() const { return m_outline_mul; }
void OutlineShader::set_jitter_amp(float amp) { m_jitter_amp = amp; }
float OutlineShader::get_jitter_amp() const { return m_jitter_amp; }
void OutlineShader::set_jitter_freq(float freq) { m_jitter_freq = freq; }
float OutlineShader::get_jitter_freq() const { return m_jitter_freq; }
void OutlineShader::set_dt(double dt) { m_dt = (float)dt; }
float OutlineShader::get_dt() const { return m_dt; }
void OutlineShader::set_jitter(bool jitter) { m_jitter_toggle = jitter; }
bool OutlineShader::get_jitter() const { return m_jitter_toggle; }