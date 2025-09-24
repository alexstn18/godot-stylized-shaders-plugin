#include "base_shader.hpp"
#include <godot_cpp/classes/rd_shader_file.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>

void BaseShader::_bind_methods() {}

void BaseShader::construct()
{
    set_effect_callback_type(CompositorEffect::EFFECT_CALLBACK_TYPE_POST_TRANSPARENT);

    m_shader = RID();
    m_pipeline = RID();
    set_enabled(false);
}

void BaseShader::queue_callable_on_render_thread(const Callable &c)
{
    if (auto *rs = RenderingServer::get_singleton())
    {
        rs->call_on_render_thread(c);
        UtilityFunctions::print("Queued function on render thread");
    }
}

void BaseShader::init_compute(const String &shader_filename)
{
    m_device = RenderingServer::get_singleton()->get_rendering_device();
    ERR_FAIL_COND_MSG(!m_device, "No device");
 
    String shader_path = m_addon_path + shader_filename;
    Ref<RDShaderFile> shader_file = ResourceLoader::get_singleton()->load(shader_path);
    ERR_FAIL_COND_MSG(!shader_file.is_valid(), "Failed to load shader file!");
    
    String base_error = shader_file->get_base_error();
    ERR_FAIL_COND_MSG(!base_error.is_empty(), "Shader compilation error: " + base_error);
    
    Ref<RDShaderSPIRV> spirv = shader_file->get_spirv();
    ERR_FAIL_COND_MSG(!spirv.is_valid(), "Failed to get SPIRV from shader file!");
    
    m_shader = m_device->shader_create_from_spirv(spirv);
    ERR_FAIL_COND_MSG(!m_shader.is_valid(), "Failed to create shader from SPIRV!");
    
    m_pipeline = m_device->compute_pipeline_create(m_shader);
    UtilityFunctions::print("Shader and pipeline created successfully");
}

void BaseShader::free_shader()
{
    if (m_shader.is_valid())
    {
        m_device->free_rid(m_shader);
        m_shader = RID();
        UtilityFunctions::print("Freed shader");
    }
    if (m_pipeline.is_valid())
    {
        m_device->free_rid(m_pipeline);
        m_shader = RID();
        UtilityFunctions::print("Freed pipeline");
    }
}