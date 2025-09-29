#include "slider_container.hpp"

// #define CONTROL_QUEUE_FREE(T) if(T) { T->queue_free(); T = nullptr; }

void SliderContainer::_bind_methods() {}

SliderContainer::SliderContainer()
{
    m_label = memnew(Label);
    m_slider = memnew(HSlider);

    add_child(m_label);
    add_child(m_slider);
}

SliderContainer::~SliderContainer() {}

void SliderContainer::set_label_text(const String &text)
{
    if(m_label) m_label->set_text(text);
}

void SliderContainer::connect_to_slider(const Callable &c)
{
    if(m_slider) m_slider->connect("value_changed", c);
}

void SliderContainer::set_slider_step(const double step) 
{
    if(m_slider) m_slider->set_step(step);
}

void SliderContainer::set_slider_min(const double min) 
{
    if(m_slider) m_slider->set_min(min);
}

void SliderContainer::set_slider_max(const double max) 
{
    if(m_slider) m_slider->set_max(max);
}