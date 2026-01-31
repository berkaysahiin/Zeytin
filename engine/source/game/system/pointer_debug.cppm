module;

#include "preparser.h"
#include "raylib.h"

export module zeytin.game.pointer_debug;
import zeytin.component;
import zeytin.zeytin;
import zeytin.raylib;

export struct CPointerDebug final : public Component
{
    PROPERTY(GROUP="Debug")
    bool enabled = true;

    PROPERTY(GROUP="Debug")
    float radius = 8.0F;

    PROPERTY(GROUP="Debug")
    int color_r = 255;

    PROPERTY(GROUP="Debug")
    int color_g = 80;

    PROPERTY(GROUP="Debug")
    int color_b = 80;

    PROPERTY(GROUP="Debug")
    int color_a = 220;

    void on_update() override {
        if (!enabled) {
            return;
        }

        const Vector2 mouse_pos = get_mouse_position();
        const Camera2D& camera = Zeytin::get().get_camera();
        const Vector2 world_mouse = get_screen_to_world2d(mouse_pos, camera);
        const Color color = Color{
            .r=static_cast<unsigned char>(color_r),
            .g=static_cast<unsigned char>(color_g),
            .b=static_cast<unsigned char>(color_b),
            .a=static_cast<unsigned char>(color_a)
        };

        DrawCircleV(world_mouse, radius, color);
        DrawCircleLines(static_cast<int>(world_mouse.x), static_cast<int>(world_mouse.y), radius + 2.0F, Color{.r=255, .g=255, .b=255, .a=120});
    }
};
