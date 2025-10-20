#pragma once

#include "encapsulated_data.hpp"
#include <godot_cpp/classes/node.hpp>

using namespace godot;

// None of the allocated memory (using "memnew") needs to be freed using this
// As in Godot's node trees, once you delete the parent the children get
// automatically freed too

class SliderContainer;

template <typename T> class NodeBuilder
{
    T *node = nullptr;

  public:
    NodeBuilder(T *n) : node(n) {}

    template <typename Func, typename... Args>
    NodeBuilder<T> &call(Func func, Args &&...args)
    {
        static_assert(std::is_member_function_pointer_v<Func>,
                      "call() can only be used with member function pointers");
        (node->*func)(std::forward<Args>(args)...);
        return *this;
    }

    template <class C> NodeBuilder<C> add_child()
    {
        static_assert(std::is_base_of_v<Node, C>,
                      "add_child() can only be used to add Node children");
        C *child = memnew(C);
        node->add_child(child);
        return NodeBuilder<C>(child);
    }

    NodeBuilder<SliderContainer> &slider_container_init(
        const String &name, const String &label_text, double step, double min,
        double max, EncapsuledData<float> *value, const Callable &callable)
    {
        static_assert(std::is_same_v<T, SliderContainer>,
                      "slider_container_init() can only be used on "
                      "SliderContainer builders");
        node->set_name(name);
        node->set_label_text(label_text);
        node->set_slider_step(step);
        node->set_slider_min(min);
        node->set_slider_max(max);
        node->set_slider_value(value->get());
        value->connect_slider(get());
        node->connect_to_slider(callable);
        return *this;
    }

    static NodeBuilder<T> create(Node *parent = nullptr)
    {
        static_assert(std::is_base_of_v<Node, T>,
                      "create() can only be used on Node builders");
        T *node = memnew(T);
        if (parent)
            parent->add_child(node);
        return NodeBuilder<T>(node);
    }

    template <typename C> NodeBuilder<T> &remove_child(NodeBuilder<C> &builder)
    {
        static_assert(std::is_base_of_v<Node, C>,
                      "remove_child() can only be used on Node builders");

        if (Node *child = builder.get(); child->get_parent() == node)
        {
            node->remove_child(builder.get());
        }
        return *this;
    }

    template <typename C> NodeBuilder<T> &try_remove_child(C *child)
    {
        static_assert(std::is_base_of_v<Node, C>,
                      "remove_child() can only be used on Node children");

        if (child && child->get_parent() == node)
        {
            node->remove_child(child);
        }
        return *this;
    }

    T *get() const { return node; }
    operator T *() const { return node; }
};