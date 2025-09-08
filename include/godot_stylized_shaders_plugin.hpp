#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

class godot_stylized_shaders_plugin : public Object 
{
    GDCLASS(godot_stylized_shaders_plugin, Object);

protected:
    static void _bind_methods();
public:
    static float lerp(float a, float b, float t);
    static float inverse_lerp(float a, float b, float v);
    static int sum(const Array& values);
    static Array get_key_values(const Array& values, const String& key);
};