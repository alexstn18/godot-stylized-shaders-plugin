#include "cel_shader.hpp"
#include "godot_cpp/classes/compositor_effect.hpp"
#include <godot_cpp/classes/render_scene_buffers_rd.hpp>

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

            /*
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
            */
        }
    }
}