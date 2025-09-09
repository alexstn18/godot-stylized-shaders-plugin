#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/classes/compositor.hpp>

#include "grayscale_effect.hpp"

using namespace godot;

// TODO: https://docs.godotengine.org/en/latest/tutorials/rendering/compositor.html
// TODO: https://godotforums.org/d/32073-debug-c-gdextension/17

class godot_stylized_shaders_plugin : public Node
{
    GDCLASS(godot_stylized_shaders_plugin, Node);

private:
    Ref<GrayscaleEffect> m_grayscaleEffect;
    Compositor* m_compositor = nullptr;

    void setup();

protected:
    static void _bind_methods();
public:
    static float lerp(float a, float b, float t);
    static float inverse_lerp(float a, float b, float v);
    static int sum(const Array& values);
    static Array get_key_values(const Array& values, const String& key);

    void _ready() override;
    void _process(double delta) override;
    void _notification(int what);
};