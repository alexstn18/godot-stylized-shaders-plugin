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
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>

// Converted to C++ GDExtension from:
// https://docs.godotengine.org/en/latest/tutorials/rendering/compositor.html

void PostProcessShader::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_shader_code", "value"), &PostProcessShader::set_shader_code);
    ClassDB::bind_method(D_METHOD("get_shader_code"), &PostProcessShader::get_shader_code);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "m_shader_code", PROPERTY_HINT_MULTILINE_TEXT),
                "set_shader_code", "get_shader_code");
}

PostProcessShader::PostProcessShader()
{
    set_effect_callback_type(CompositorEffect::EFFECT_CALLBACK_TYPE_POST_TRANSPARENT);
}

PostProcessShader::~PostProcessShader()
{
    if (m_device)
    {
        if (m_shader.is_valid())
        {
            m_device->free_rid(m_shader);
            m_shader = RID();
        }
        if (m_pipeline.is_valid())
        {
            m_device->free_rid(m_pipeline);
            m_pipeline = RID();
        }
    } 
    else 
    {
        m_shader = RID();
        m_pipeline = RID();
    }
}

void PostProcessShader::_notification(int what)
{
    if (what == NOTIFICATION_PREDELETE)
    {
        if (m_device)
        {
            if (m_shader.is_valid())
            {
                m_device->free_rid(m_shader);
                m_shader = RID();
            }
            if (m_pipeline.is_valid())
            {
                m_device->free_rid(m_pipeline);
                m_pipeline = RID();
            }
        }
        else
        {
            m_shader = RID();
            m_pipeline = RID();
        }
    }
}


bool PostProcessShader::_check_shader()
{

    if (!m_device)
    {
        if (RenderingServer::get_singleton())
        {
            m_device = RenderingServer::get_singleton()->get_rendering_device();
        }
    }
    if (!m_device) return false;

    String new_shader_code = "";

    m_mutex.lock();
    if (m_shader_dirty)
    {
        new_shader_code = m_shader_code;
        m_shader_dirty = false;
    }
    m_mutex.unlock();

    if (new_shader_code.is_empty()) return m_pipeline.is_valid();
    new_shader_code = m_shader_template.replace("#COMPUTE_CODE", new_shader_code);

    if (m_shader.is_valid())
    {
        if (m_device)
        {
            m_device->free_rid(m_shader);
        }
        m_shader = RID();

        if (m_device && m_pipeline.is_valid()) {
            m_device->free_rid(m_pipeline);
        }
        m_pipeline = RID();
    }

    Ref<RDShaderSource> source;
    Ref<RDShaderSPIRV> spirv;
    source.instantiate();
    spirv.instantiate();

    source->set_language(RenderingDevice::SHADER_LANGUAGE_GLSL);
    source->set_stage_source(RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE, 
                             new_shader_code);

    spirv = m_device->shader_compile_spirv_from_source(source);
    if(spirv->get_stage_compile_error(RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE) != "")
    {
        UtilityFunctions::push_error(spirv->get_stage_compile_error(RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE));
        UtilityFunctions::push_error("In: " + new_shader_code);
        return false;
    }

    m_shader = m_device->shader_create_from_spirv(spirv);
    if(!m_shader.is_valid()) return false;

    m_pipeline = m_device->compute_pipeline_create(m_shader);
    return m_pipeline.is_valid();
}

void PostProcessShader::_render_callback(int32_t p_effect_callback_type,
                                         RenderData *p_render_data)
{
    if(m_device && p_effect_callback_type == EFFECT_CALLBACK_TYPE_POST_TRANSPARENT && _check_shader())
    {
        Ref<RenderSceneBuffersRD> buffers = p_render_data->get_render_scene_buffers();
        if(buffers.is_valid())
        {
            Vector2i size = buffers->get_internal_size();
            if(size.x == 0 && size.y == 0) return;

            int x_groups = (size.x - 1) / 8 + 1;
            int y_groups = (size.y - 1) / 8 + 1;
            int z_groups = 1;

            PackedFloat32Array push_constant = PackedFloat32Array();
            push_constant.push_back(size.x);
            push_constant.push_back(size.y);
            push_constant.push_back(0);
            push_constant.push_back(0);

            uint32_t view_count = buffers->get_view_count();
            for(uint32_t i = 0; i < view_count; i++)
            {
                RID input_image = buffers->get_color_layer(i);
                Ref<RDUniform> uniform;
                uniform.instantiate();
                uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_IMAGE);
                uniform->set_binding(0);
                uniform->add_id(input_image);
                RID uniform_set = UniformSetCacheRD::get_cache(m_shader, 0, {uniform});

                auto compute_list = m_device->compute_list_begin();
                m_device->compute_list_bind_compute_pipeline(compute_list, m_pipeline);
                m_device->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
                m_device->compute_list_set_push_constant(compute_list, push_constant.to_byte_array(), push_constant.size() * 4); // 4 = sizeof(Float32)
                m_device->compute_list_dispatch(compute_list, x_groups, y_groups, z_groups);
                m_device->compute_list_end();
            }
        }
    }
}

void PostProcessShader::set_shader_code(const String& value)
{
    m_mutex.lock();
    m_shader_code = value;
    m_shader_dirty = true;
    m_mutex.unlock();
}

String PostProcessShader::get_shader_code() const { return m_shader_code; }
