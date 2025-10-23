#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "base_shader.hpp"
#include "util/encapsulated_data.hpp"

using namespace godot;

class KuwaharaShader : public BaseShader
{
    GDCLASS(KuwaharaShader, BaseShader);

private:
    RID m_structure_tensor_shader;
    RID m_horizontal_blur_shader;
    RID m_vertical_blur_shader;
    RID m_composite_shader;
    RID m_structure_tensor_pipeline;
    RID m_horizontal_blur_pipeline;
    RID m_vertical_blur_pipeline;
    RID m_composite_pipeline;


protected:
    static void _bind_methods();
public:
    KuwaharaShader();
    ~KuwaharaShader();
    EncapsuledData<float> *m_radius = memnew(EncapsuledData<float>(1.0f));

    void _notification(int what) override;
    void _render_callback(int32_t, RenderData *) override;
};