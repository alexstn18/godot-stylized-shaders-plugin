#pragma once

#include <godot_cpp/classes/compositor_effect.hpp>

class StylizedEffect : public godot::CompositorEffect
{
    GDCLASS(StylizedEffect, CompositorEffect);

private:
    bool m_toggle = false;

public:
    void set_toggle(const bool toggle) { m_toggle = toggle; }
    bool get_toggle() const { return m_toggle; }
};