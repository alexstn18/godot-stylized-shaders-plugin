#include "bloom_shader.hpp"
#include "godot_cpp/classes/rd_sampler_state.hpp"
#include "godot_cpp/classes/render_scene_buffers_rd.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/method_bind.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"

#include <godot_cpp/classes/compositor_effect.hpp>
#include <godot_cpp/classes/rd_shader_file.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/render_scene_data_rd.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>

void BloomShader::_bind_methods() {}

BloomShader::BloomShader()
{
    set_effect_callback_type(
        CompositorEffect::EFFECT_CALLBACK_TYPE_POST_TRANSPARENT);
    set_enabled(false);

    m_downsample_shader = RID();
    m_upsample_shader = RID();
    m_add_shader = RID();
    m_downsample_pipeline = RID();
    m_upsample_pipeline = RID();
    m_add_pipeline = RID();
    m_bilinear_sampler = RID();

    queue_callable_on_render_thread(
        callable_mp(this, &BloomShader::init_compute));
}

BloomShader::~BloomShader() {}

void BloomShader::init_compute()
{
    m_device = RenderingServer::get_singleton()->get_rendering_device();
    ERR_FAIL_COND_MSG(!m_device, "No device");

    String addon_path = get_addon_path();
    String downsample_path = addon_path + "bloom_downsample.glsl";
    String upsample_path = addon_path + "bloom_upsample.glsl";
    String add_path = addon_path + "bloom_add.glsl";

    create_shader(downsample_path, m_downsample_shader, m_downsample_pipeline);
    create_shader(upsample_path, m_upsample_shader, m_upsample_pipeline);
    create_shader(add_path, m_add_shader, m_add_pipeline);

    Ref<RDSamplerState> sampler_state;
    sampler_state.instantiate();
    sampler_state->set_min_filter(RenderingDevice::SAMPLER_FILTER_LINEAR);
    sampler_state->set_mag_filter(RenderingDevice::SAMPLER_FILTER_LINEAR);
    m_bilinear_sampler = m_device->sampler_create(sampler_state);
    ERR_FAIL_COND_MSG(!m_bilinear_sampler.is_valid(),
                      "Failed to create bilinear sampler!");
}

void BloomShader::_notification(int what)
{
    if (what == NOTIFICATION_PREDELETE && m_device)
    {
        free_rid(m_downsample_shader);
        free_rid(m_upsample_shader);
        free_rid(m_add_shader);
        free_rid(m_bilinear_sampler);
    }
}

void BloomShader::_render_callback(int32_t p_effect_callback_type,
                                   RenderData *p_render_data)
{
    if (m_device && m_downsample_pipeline.is_valid() &&
        m_upsample_pipeline.is_valid() && m_add_pipeline.is_valid() &&
        p_effect_callback_type == EFFECT_CALLBACK_TYPE_POST_TRANSPARENT)
    {
        Ref<RenderSceneBuffersRD> buffers;
        buffers.instantiate();

        Vector2i size = get_buffers_internal_size(p_render_data, buffers);
        ERR_FAIL_COND_MSG(size.x == 0 || size.y == 0, "Buffer size is 0");
        RenderSceneData *scene_data = p_render_data->get_render_scene_data();
        if (buffers.is_valid() || !scene_data)
        {
            auto usage = RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | RenderingDevice::TEXTURE_USAGE_STORAGE_BIT;
            buffers->create_texture("Bloom", "Downsample", RenderingDevice::DATA_FORMAT_R16G16B16A16_SFLOAT, usage, RenderingDevice::TEXTURE_SAMPLES_1, size, 1, 1, true, false);
	        buffers->create_texture("Bloom", "Upsample", RenderingDevice::DATA_FORMAT_R16G16B16A16_SFLOAT, usage, RenderingDevice::TEXTURE_SAMPLES_1, size, 1, 1, true, false);

            PackedFloat32Array downsample_push_constant = {float(size.x), float(size.y), m_threshold, 0.0f}; // replace 1.0f with bloom threshold
            PackedFloat32Array upsample_push_constant = {float(size.x), float(size.y), m_radius, 0.0f}; // replace 1.0f with bloom radius
            PackedFloat32Array add_push_constant = {float(size.x), float(size.y), 0.0f, 0.0f};

            const int x_groups = (size.x + 15) / 16;
            const int y_groups = (size.y + 15) / 16;

            auto view_count = buffers->get_view_count();
            for (auto i = 0; i < view_count; ++i)
            {
                RID input_image = buffers->get_color_layer(i);
                auto downsample_texture = buffers->get_texture_slice("Bloom", "Downsample", i, 0, 1, 1);
                auto upsample_texture = buffers->get_texture_slice("Bloom", "Upsample", i, 0, 1, 1);

                // Downsample
                auto compute_list = m_device->compute_list_begin();
                m_device->compute_list_bind_compute_pipeline(compute_list, m_downsample_pipeline);
                auto downsample_in = get_sampler_uniform(input_image, 0);
                auto downsample_out = get_image_uniform(downsample_texture, 1);
                RID uniform_set = UniformSetCacheRD::get_cache(m_downsample_shader, 0, { downsample_in, downsample_out });

                m_device->compute_list_bind_uniform_set(compute_list, uniform_set, 0);

                m_device->compute_list_set_push_constant(compute_list, downsample_push_constant.to_byte_array(), downsample_push_constant.size() * 4);

                m_device->compute_list_dispatch(compute_list, x_groups, y_groups, 1);
                m_device->compute_list_end();

                // Upsample
                compute_list = m_device->compute_list_begin();
                m_device->compute_list_bind_compute_pipeline(compute_list, m_upsample_pipeline);
                auto upsample_in = get_sampler_uniform(downsample_texture, 0);
                auto upsample_out = get_image_uniform(upsample_texture, 1);
                uniform_set = UniformSetCacheRD::get_cache(m_upsample_shader, 0, { upsample_in, upsample_out });
                m_device->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
                m_device->compute_list_set_push_constant(compute_list, upsample_push_constant.to_byte_array(), upsample_push_constant.size() * 4);
                m_device->compute_list_dispatch(compute_list, x_groups, y_groups, 1);
                m_device->compute_list_end();

                // Add
                compute_list = m_device->compute_list_begin();
                m_device->compute_list_bind_compute_pipeline(compute_list, m_add_pipeline);
                auto add_in1 = get_sampler_uniform(input_image, 0);
                auto add_in2 = get_sampler_uniform(upsample_texture, 1);
                uniform_set = UniformSetCacheRD::get_cache(m_add_shader, 0, { add_in1, add_in2 });
                m_device->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
                m_device->compute_list_set_push_constant(compute_list, add_push_constant.to_byte_array(), add_push_constant.size() * 4);
                m_device->compute_list_dispatch(compute_list, x_groups, y_groups, 1);
                m_device->compute_list_end();
            }
        }
    }
}

void BloomShader::create_shader(const String &shader_path, RID &shader,
                                RID &pipeline)
{
    Ref<RDShaderFile> shader_file =
        ResourceLoader::get_singleton()->load(shader_path);
    ERR_FAIL_COND_MSG(!shader_file.is_valid(), "Failed to load shader file!");

    String base_error = shader_file->get_base_error();
    ERR_FAIL_COND_MSG(!base_error.is_empty(),
                      "Shader compilation error: " + base_error);

    Ref<RDShaderSPIRV> spirv = shader_file->get_spirv();
    ERR_FAIL_COND_MSG(!spirv.is_valid(),
                      "Failed to get SPIRV from shader file!");

    shader = m_device->shader_create_from_spirv(spirv);
    ERR_FAIL_COND_MSG(!shader.is_valid(),
                      "Failed to create shader from SPIRV!");

    pipeline = m_device->compute_pipeline_create(shader);
    UtilityFunctions::print("Shader and pipeline created successfully");
}

void BloomShader::free_rid(RID &rid)
{
    if (rid.is_valid())
    {
        m_device->free_rid(rid);
        rid = RID();
        UtilityFunctions::print("Freed RID");
    }
}

Ref<RDUniform> BloomShader::get_sampler_uniform(const RID &image,
                                                int32_t binding)
{
    Ref<RDUniform> uniform;
    uniform.instantiate();

    uniform->set_uniform_type(
        RenderingDevice::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE);
    uniform->set_binding(binding);
    uniform->add_id(m_bilinear_sampler);
    uniform->add_id(image);

    return uniform;
}

Ref<RDUniform> BloomShader::get_image_uniform(const RID &image, int32_t binding)
{
    Ref<RDUniform> uniform;
    uniform.instantiate();

    uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_IMAGE);
    uniform->set_binding(binding);
    uniform->add_id(image);

    return uniform;
}

Ref<RDUniform> BloomShader::get_buffer_uniform(const RID &buffer, int binding)
{
    Ref<RDUniform> uniform;
    uniform.instantiate();

    uniform->set_uniform_type(RenderingDevice::UNIFORM_TYPE_STORAGE_BUFFER);
    uniform->set_binding(binding);
    uniform->add_id(buffer);

    return uniform;
}
