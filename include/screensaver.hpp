#pragma once

#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot
{
    class Screensaver : public Sprite2D 
    {
        GDCLASS(Screensaver, Sprite2D);

    private:
        Vector2 screen_size;
        Vector2 sprite_size;

        int speed = 200;
        int x_direction = 1;
        int y_direction = 1;

        void set_speed(const int value);
        int get_speed();
    protected:
        static void _bind_methods();
    public:
        void _ready() override;
        void _process(double delta) override;
    };
}