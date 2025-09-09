#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/compositor.hpp>
#include "stylized_effect.hpp"

using namespace godot;

class GrayscaleEffect : public StylizedEffect 
{
    GDCLASS(GrayscaleEffect, StylizedEffect);

private:
    
protected:
    static void _bind_methods();
public:

};