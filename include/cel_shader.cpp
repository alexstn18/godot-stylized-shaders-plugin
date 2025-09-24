#include "cel_shader.hpp"
#include "godot_cpp/classes/compositor_effect.hpp"
#include <godot_cpp/classes/render_scene_buffers_rd.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>

void CelShader::_bind_methods()
{
    
}

CelShader::CelShader()
{
    construct();

    queue_callable_on_render_thread(callable_mp(this, &CelShader::init_compute).bind("cel.glsl"));
}

CelShader::~CelShader() {}

void CelShader::init_compute(const String &shader_filename)
{
    BaseShader::init_compute(shader_filename);
    // ...
}

void CelShader::_notification(int what)
{
    if(what == NOTIFICATION_PREDELETE && m_device)
    {
        free_shader();
    }
}

void CelShader::_render_callback(int32_t p_effect_callback_type, RenderData *p_render_data)
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

            PackedFloat32Array push_constant = {(float)size.x,
                                                (float)size.y,
                                                (float)m_levels, 
                                                0.0f};
            ERR_FAIL_COND_MSG(push_constant.is_empty(), "push constant is empty");

            uint32_t view_count = buffers->get_view_count();
            
            for(int32_t i = 0; i < view_count; i++)
            {
                RID input_image = buffers->get_color_layer(i);
                ERR_CONTINUE_MSG(!input_image.is_valid(), "Invalid input image for view " + String::num(i));

                Ref<RDUniform> uniform; uniform.instantiate();
                uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_IMAGE);
                uniform->set_binding(0);
                uniform->add_id(input_image);
                
                RID image_uniform_set = UniformSetCacheRD::get_cache(m_shader, 0, {uniform});
                ERR_CONTINUE_MSG(!image_uniform_set.is_valid(), "Failed to create color image uniform set for view " + String::num(i));

                uniform.instantiate();
                uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE);
                uniform->set_binding(0);

                auto compute_list = m_device->compute_list_begin();
                m_device->compute_list_bind_compute_pipeline(compute_list, m_pipeline);
                m_device->compute_list_bind_uniform_set(compute_list, image_uniform_set, 0);
                m_device->compute_list_set_push_constant(compute_list, push_constant.to_byte_array(), push_constant.size() * 4);
                m_device->compute_list_dispatch(compute_list, x_groups, y_groups, 1);
                m_device->compute_list_end();
            }
        }
    }
}

void CelShader::set_levels(int levels) { m_levels = levels; }
int CelShader::get_levels() const { return m_levels; }