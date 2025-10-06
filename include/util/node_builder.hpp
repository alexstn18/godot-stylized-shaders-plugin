#pragma once

#include <godot_cpp/classes/node.hpp>

using namespace godot;

// None of the allocated memory (using "memnew") needs to be freed using this
// As in Godot's node trees, once you delete the parent the children get automatically freed too

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

    static NodeBuilder<T> create(Node *parent = nullptr)
    {
        T *node = memnew(T);
        if(parent) parent->add_child(node);
        return NodeBuilder<T>(node);
    }

    T *get() const { return node; }
    operator T*() const { return node; }
};