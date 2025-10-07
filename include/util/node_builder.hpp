#pragma once

#include <godot_cpp/classes/node.hpp>

using namespace godot;

// None of the allocated memory (using "memnew") needs to be freed using this
// As in Godot's node trees, once you delete the parent the children get automatically freed too

class SliderContainer;

template <typename T>
class NodeBuilder
{
    T *node = nullptr;
public:
    NodeBuilder(T *n) : node(n) {}

    template <typename Func, typename... Args>
    NodeBuilder<T> &call(Func func, Args&&... args)
    {
        (node->*func)(std::forward<Args>(args)...);
        return *this;
    }

    template <class C>
    NodeBuilder<C> add_child()
    {
        C *child = memnew(C);
        node->add_child(child);
        return NodeBuilder<C>(child);
    }

    NodeBuilder<SliderContainer> &slider_container_init(const String &name, const String &label_text, double step, double min, double max, double value, const Callable &callable)
    {
        static_assert(std::is_same_v<T, SliderContainer>,
              "slider_container_init() can only be used on SliderContainer builders");
        node->set_name(name);
        node->set_label_text(label_text);
        node->set_slider_step(step);
        node->set_slider_min(min);
        node->set_slider_max(max);
        node->set_slider_value(value);
        node->connect_to_slider(callable);
        return *this;
    }

    static NodeBuilder<T> create(Node *parent = nullptr)
    {
        T *node = memnew(T);
        if(parent) parent->add_child(node);
        return NodeBuilder<T>(node);
    }

    T *get() const { return node; }
    operator T*() const { return node; }
};