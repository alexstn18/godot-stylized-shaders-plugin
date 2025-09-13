#include "post_process_shader.hpp"
#include "godot_cpp/classes/compositor_effect.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
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

// Converted to C++ GDExtension from:
// https://docs.godotengine.org/en/latest/tutorials/rendering/compositor.html

void PostProcessShader::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("init_compute"), &PostProcessShader::init_compute);
}

PostProcessShader::PostProcessShader()
{
    set_effect_callback_type(CompositorEffect::EFFECT_CALLBACK_TYPE_POST_TRANSPARENT);

    m_mutex.instantiate();

    if (auto *rs = RenderingServer::get_singleton())
    {
        auto c = Callable(this, "init_compute");
        rs->call_on_render_thread(c);
    }
}

PostProcessShader::~PostProcessShader() {}

void PostProcessShader::_notification(int what)
{
    if (what == NOTIFICATION_PREDELETE && m_device)
    {
        if (m_shader.is_valid())
        {
            m_device->free_rid(m_shader); // supposedly frees pipeline too?
            m_shader = RID();
            m_pipeline = RID();
        }
    }
}

void PostProcessShader::_render_callback(int32_t p_effect_callback_type,
                                         RenderData *p_render_data)
{
    if(m_device && 
        p_effect_callback_type == EFFECT_CALLBACK_TYPE_POST_TRANSPARENT)
    {
        Ref<RenderSceneBuffersRD> buffers;
        buffers.instantiate();
        buffers = p_render_data->get_render_scene_buffers();
        if(buffers.is_valid())
        {
            Vector2i size = buffers->get_internal_size();

            if(size.x == 0 || size.y == 0) return;

            const int x_groups = size.x / 16;
            const int y_groups = size.y / 16;

            PackedFloat32Array push_constant = PackedFloat32Array();
            push_constant.push_back(size.x);
            push_constant.push_back(size.y);
            push_constant.push_back(0);
            push_constant.push_back(0);

            uint32_t view_count = buffers->get_view_count();
            for(auto i = 0; i < view_count; i++)
            {
                RID input_image = buffers->get_color_layer(i);
                Ref<RDUniform> uniform;
                TypedArray<Ref<RDUniform>> uniform_array;
                
                uniform.instantiate();
                uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_IMAGE);
                uniform->set_binding(0);
                uniform->add_id(input_image);
                uniform_array.push_back(uniform);
                
                RID uniform_set = UniformSetCacheRD::get_cache(m_shader, 0, uniform_array);

                auto compute_list = m_device->compute_list_begin();
                m_device->compute_list_bind_compute_pipeline(compute_list, m_pipeline);
                m_device->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
                m_device->compute_list_set_push_constant(compute_list, push_constant.to_byte_array(), push_constant.size() * 4); // 4 = sizeof(Float32)
                m_device->compute_list_dispatch(compute_list, x_groups, y_groups, 1);
                m_device->compute_list_end();
            }
        }
    }
}

void PostProcessShader::init_compute()
{
    m_device = RenderingServer::get_singleton()->get_rendering_device();
    if(!m_device) return;
    
    Ref<RDShaderFile> glsl_file;
    glsl_file.instantiate();
    glsl_file = ResourceLoader::get_singleton()->load("res://addons/GodotStylizedShadersPlugin/shaders/compute_template.glsl");
    // UtilityFunctions::push_error(glsl_file->get_base_error());

    m_shader = m_device->shader_create_from_spirv(glsl_file->get_spirv());
    m_pipeline = m_device->compute_pipeline_create(m_shader);
}