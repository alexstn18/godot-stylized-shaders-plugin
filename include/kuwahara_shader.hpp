#pragma once

#include "base_shader.hpp"
#include "util/encapsulated_data.hpp"
#include <cstdint>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// Default presets
enum class KuwaharaPreset : uint8_t
{
    SUBTLE = 0U,
    OIL,
    ANIME
};

struct KuwaharaPresetConfig
{
    String label_text;
    float radius;
    float kernel_size;
    float alpha;
    float sectors;
    float sharpness;
};

class KuwaharaShader : public BaseShader
{
    GDCLASS(KuwaharaShader, BaseShader);

  private:
    RID m_structure_tensor_shader;
    RID m_horizontal_blur_shader;
    RID m_vertical_blur_shader;
    RID m_composite_shader;
    RID m_final_shader;
    RID m_structure_tensor_pipeline;
    RID m_horizontal_blur_pipeline;
    RID m_vertical_blur_pipeline;
    RID m_composite_pipeline;
    RID m_final_pipeline;
    RID m_billinear_sampler;

    float m_zero_crossing = 1.6F;

    KuwaharaPreset m_preset = KuwaharaPreset::SUBTLE;

    std::vector<KuwaharaPresetConfig> m_preset_configs;

    void apply_config(const KuwaharaPresetConfig &config);

    void init_compute();

  protected:
    static void _bind_methods();

  public:
    KuwaharaShader();
    ~KuwaharaShader();
    EncapsuledData<float> *radius = memnew(EncapsuledData<float>(3.0f));
    EncapsuledData<float> *kernel_size = memnew(EncapsuledData<float>(7.0f));
    EncapsuledData<float> *alpha = memnew(EncapsuledData<float>(1.0f));
    EncapsuledData<float> *sectors = memnew(EncapsuledData<float>(8.0f));
    EncapsuledData<float> *sharpness = memnew(EncapsuledData<float>(8.0f));

    void _notification(int what) override;
    void _render_callback(int32_t, RenderData *) override;

    KuwaharaPreset get_preset() const { return m_preset; }
    void set_preset(KuwaharaPreset p);
    void set_preset(const KuwaharaPresetConfig &config);

    const std::vector<KuwaharaPresetConfig> &get_preset_configs() const
    {
        return m_preset_configs;
    }
    void add_preset_config(const KuwaharaPresetConfig &config);
};