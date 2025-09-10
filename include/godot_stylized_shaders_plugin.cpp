#include "godot_stylized_shaders_plugin.hpp"

#include "mathf.hpp"
#include "convert.hpp"

// #include <godot_cpp/classes/viewport.hpp>
// #include <godot_cpp/classes/file_access.hpp>
// #include <godot_cpp/variant/string.hpp>

void godot_stylized_shaders_plugin::_bind_methods()
{
    // GodotCon Scott Doxey examples
    ClassDB::bind_static_method("godot_stylized_shaders_plugin", D_METHOD("lerp", "a", "b", "t"), &godot_stylized_shaders_plugin::lerp);
    ClassDB::bind_static_method("godot_stylized_shaders_plugin", D_METHOD("inverse_lerp", "a", "b", "v"), &godot_stylized_shaders_plugin::inverse_lerp);
    ClassDB::bind_static_method("godot_stylized_shaders_plugin", D_METHOD("sum", "values"), &godot_stylized_shaders_plugin::sum);
    ClassDB::bind_static_method("godot_stylized_shaders_plugin", D_METHOD("get_key_values", "values", "key"), &godot_stylized_shaders_plugin::get_key_values);
    
    // My implementations
    ClassDB::bind_method(D_METHOD("setup"), &godot_stylized_shaders_plugin::setup);
}

void godot_stylized_shaders_plugin::_ready()
{
}

void godot_stylized_shaders_plugin::_process(double delta)
{
    
}

void godot_stylized_shaders_plugin::_notification(int what)
{
    
}

void godot_stylized_shaders_plugin::setup()
{
    // String template_shader = FileAccess::get_file_as_string("/shaders/compute_template.gdshader");

}

float godot_stylized_shaders_plugin::lerp(float a, float b, float t)
{
    return Mathf::lerp(a, b, t);
}

float godot_stylized_shaders_plugin::inverse_lerp(float a, float b, float v)
{
    return Mathf::inverse_lerp(a, b, v);
}

int godot_stylized_shaders_plugin::sum(const Array& values)
{
    return Mathf::sum(values);
}

Array godot_stylized_shaders_plugin::get_key_values(const Array& values, const String& key)
{
    return Convert::get_key_values(values, key);
}