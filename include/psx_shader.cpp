#include "psx_shader.hpp"

void PSXShader::_bind_methods() {}

PSXShader::PSXShader()
{
    construct();
    queue_callable_on_render_thread(callable_mp(this, &PSXShader::init_compute).bind("psx.glsl"));
}

void PSXShader::init_compute(const String &shader_filename)
{
    BaseShader::init_compute(shader_filename);
}

void PSXShader::_notification(int what)
{
    UtilityFunctions::print("PostProcessShader notification: " + String::num(what));
    
    if (what == NOTIFICATION_PREDELETE && m_device)
    {
        free_shader();
    }
}

void PSXShader::_render_callback(int32_t p_effect_callback_type, RenderData *p_render_data)
{
    Ref<RenderSceneBuffersRD> buffers; buffers.instantiate();
    Vector2i size = get_buffers_internal_size(p_render_data, buffers);
    
    ERR_FAIL_COND_MSG(size.x == 0 || size.y == 0, "Buffer size is 0");
    
    PackedFloat32Array push_constant = {(float)size.x, (float)size.y, 0.0f, 0.0f, 0.0f, 0.0f, m_dither_amount, 0.0f};
    ERR_FAIL_COND_MSG(push_constant.is_empty(), "Push constant is empty/invalid!");
    
    base_compute_update(p_effect_callback_type, p_render_data, buffers, push_constant, size);
}

void PSXShader::set_dither_amount(float dither_amount) { m_dither_amount = dither_amount; }
float PSXShader::get_dither_amount() const { return m_dither_amount; }
PSXShader::~PSXShader() {}
