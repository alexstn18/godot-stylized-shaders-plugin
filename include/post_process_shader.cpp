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

    m_shader = RID();
    m_pipeline = RID();
    set_enabled(true);
    UtilityFunctions::print("Set effect enabled to true");

    if (auto *rs = RenderingServer::get_singleton())
    {
        auto c = Callable(this, "init_compute");
        rs->call_on_render_thread(c);
        UtilityFunctions::print("Queued init_compute on render thread");
    }
}

PostProcessShader::~PostProcessShader() {}

void PostProcessShader::_notification(int what)
{
    UtilityFunctions::print("PostProcessShader notification: " + String::num(what));
    
    if (what == NOTIFICATION_PREDELETE && m_device)
    {
        UtilityFunctions::print("NOTIFICATION_PREDELETE - cleaning up");
        if (m_shader.is_valid())
        {
            m_device->free_rid(m_shader);
            m_shader = RID();
            UtilityFunctions::print("Freed shader");
        }
        
        if (m_pipeline.is_valid()) 
        {
            m_device->free_rid(m_pipeline);
            m_pipeline = RID();
            UtilityFunctions::print("Freed pipeline");
        }
    }
}

void PostProcessShader::_render_callback(int32_t p_effect_callback_type,
                                         RenderData *p_render_data)
{
    if(m_device && 
        p_effect_callback_type == EFFECT_CALLBACK_TYPE_POST_TRANSPARENT)
    {
        // Check if shader and pipeline are valid before proceeding
        if (!m_shader.is_valid())
         {
            UtilityFunctions::push_error("Shader is invalid in render callback!");
            return;
        }
        
        if (!m_pipeline.is_valid()) 
        {
            UtilityFunctions::push_error("Pipeline is invalid in render callback!");
            return;
        }
        
        Ref<RenderSceneBuffersRD> buffers;
        buffers.instantiate();
        buffers = p_render_data->get_render_scene_buffers();
        if(buffers.is_valid())
        {
            Vector2i size = buffers->get_internal_size();

            if(size.x == 0 || size.y == 0) 
            {
                UtilityFunctions::print("size is 0");
                return;
            }
            
            const int x_groups = (size.x + 15) / 16;
            const int y_groups = (size.y + 15) / 16;

            PackedFloat32Array push_constant = {(float)size.x, (float)size.y, 0, 0};

            if(push_constant.is_empty())
            {
                UtilityFunctions::push_error("push constant is empty");
                return;
            } 

            uint32_t view_count = buffers->get_view_count();
            
            for(auto i = 0; i < view_count; i++)
            {
                RID input_image = buffers->get_color_layer(i);
                if (!input_image.is_valid()) 
                {
                    UtilityFunctions::push_error("Invalid input image for view " + String::num(i));
                    continue;
                }
                
                Ref<RDUniform> uniform;
                TypedArray<Ref<RDUniform>> uniform_array;
                
                uniform.instantiate();
                uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_IMAGE);
                uniform->set_binding(0);
                uniform->add_id(input_image);
                uniform_array.push_back(uniform);
                
                RID uniform_set = UniformSetCacheRD::get_cache(m_shader, 0, uniform_array);
                if (!uniform_set.is_valid()) 
                {
                    UtilityFunctions::push_error("Failed to create uniform set for view " + String::num(i));
                    continue;
                }

                auto compute_list = m_device->compute_list_begin();
                m_device->compute_list_bind_compute_pipeline(compute_list, m_pipeline);
                m_device->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
                m_device->compute_list_set_push_constant(compute_list, push_constant.to_byte_array(), push_constant.size() * 4);
                m_device->compute_list_dispatch(compute_list, x_groups, y_groups, 1);
                m_device->compute_list_end();
            }
        }
    }
}

void PostProcessShader::init_compute()
{
    m_device = RenderingServer::get_singleton()->get_rendering_device();
    if(!m_device)
    {
        UtilityFunctions::print("No device");
        return;
    } 
    
    Ref<RDShaderFile> shader_file = ResourceLoader::get_singleton()->load("res://addons/GodotStylizedShadersPlugin/shaders/compute_template.glsl");
    
    if (!shader_file.is_valid()) {
        UtilityFunctions::push_error("Failed to load shader file!");
        return;
    }
    
    String base_error = shader_file->get_base_error();
    if (!base_error.is_empty()) {
        UtilityFunctions::push_error("Shader compilation error: " + base_error);
        return;
    }

    Ref<RDShaderSPIRV> spirv = shader_file->get_spirv();
    if (!spirv.is_valid()) {
        UtilityFunctions::push_error("Failed to get SPIRV from shader file!");
        return;
    }

    m_shader = m_device->shader_create_from_spirv(spirv);
    if (!m_shader.is_valid()) {
        UtilityFunctions::push_error("Failed to create shader from SPIRV!");
        return;
    }
    
    m_pipeline = m_device->compute_pipeline_create(m_shader);
    UtilityFunctions::print("Shader and pipeline created successfully");
}