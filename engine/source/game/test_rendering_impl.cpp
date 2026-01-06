module;

#include "raylib.h"
#include <cstdio>

module component.test_rendering;
import zeytin.raylib;

void TestRendering::on_update() {
    float dt = get_frame_time();
    float speed = move_speed;

    // modifier keys affect speed
    if (is_key_down(KEY_LEFT_SHIFT) || is_key_down(KEY_RIGHT_SHIFT)) {
        speed *= 3.0f;  // Fast mode
    }
    if (is_key_down(KEY_LEFT_CONTROL) || is_key_down(KEY_RIGHT_CONTROL)) {
        speed *= 0.3f;  // Slow mode
    }

    // toggle visibility with Space
    if (is_key_pressed(KEY_SPACE)) {
        visible = !visible;
    }

    // toggle follow mouse with F
    if (is_key_pressed(KEY_F)) {
        follow_mouse = !follow_mouse;
    }

    // toggle filled/outline with G
    if (is_key_pressed(KEY_G)) {
        is_filled = !is_filled;
    }

    // reset position with R
    if (is_key_pressed(KEY_R)) {
        position_x = 400.0f;
        position_y = 300.0f;
        width = 100.0f;
        height = 100.0f;
    }

    // mouse following mode
    if (follow_mouse) {
        Vector2 mouse = get_mouse_position();
        position_x = mouse.x - width / 2;
        position_y = mouse.y - height / 2;
    } else {
        // WASD movement
        if (is_key_down(KEY_W) || is_key_down(KEY_UP)) {
            position_y -= speed * dt;
        }
        if (is_key_down(KEY_S) || is_key_down(KEY_DOWN)) {
            position_y += speed * dt;
        }
        if (is_key_down(KEY_A) || is_key_down(KEY_LEFT)) {
            position_x -= speed * dt;
        }
        if (is_key_down(KEY_D) || is_key_down(KEY_RIGHT)) {
            position_x += speed * dt;
        }
    }

    // mouse scroll to resize
    float scroll = get_mouse_wheel_move();
    if (scroll != 0) {
        float scale_factor = 1.0f + scroll * 0.1f;
        width *= scale_factor;
        height *= scale_factor;
        // Clamp size
        if (width < 20.0f) width = 20.0f;
        if (width > 500.0f) width = 500.0f;
        if (height < 20.0f) height = 20.0f;
        if (height > 500.0f) height = 500.0f;
    }

    if (is_key_pressed(KEY_ONE)) {
        base_color_r = 1.0f; base_color_g = 0.0f; base_color_b = 0.0f;  // Red
    }
    if (is_key_pressed(KEY_TWO)) {
        base_color_r = 0.0f; base_color_g = 1.0f; base_color_b = 0.0f;  // Green
    }
    if (is_key_pressed(KEY_THREE)) {
        base_color_r = 0.0f; base_color_g = 0.0f; base_color_b = 1.0f;  // Blue
    }
    if (is_key_pressed(KEY_FOUR)) {
        base_color_r = 1.0f; base_color_g = 1.0f; base_color_b = 0.0f;  // Yellow
    }
    if (is_key_pressed(KEY_FIVE)) {
        base_color_r = 0.2f; base_color_g = 0.6f; base_color_b = 1.0f;  // Cyan (default)
    }

    float final_r = base_color_r;
    float final_g = base_color_g;
    float final_b = base_color_b;

    if (is_mouse_button_down(MOUSE_BUTTON_LEFT)) {
        final_r = 1.0f;  // Boost red channel
    }
    if (is_mouse_button_down(MOUSE_BUTTON_RIGHT)) {
        final_g = 1.0f;  // Boost green channel
    }
    if (is_mouse_button_down(MOUSE_BUTTON_MIDDLE)) {
        final_b = 1.0f;  // Boost blue channel
    }

    if (!visible) {
        DrawText("Hidden (Space to show)", 10, 10, 20, GRAY);
        return;
    }

    Color rect_color = {
        static_cast<unsigned char>(final_r * 255),
        static_cast<unsigned char>(final_g * 255),
        static_cast<unsigned char>(final_b * 255),
        static_cast<unsigned char>(color_a * 255)
    };

    if (is_filled) {
        DrawRectangle(
            static_cast<int>(position_x),
            static_cast<int>(position_y),
            static_cast<int>(width),
            static_cast<int>(height),
            rect_color
        );
    } else {
        DrawRectangleLinesEx(
            Rectangle{position_x, position_y, width, height},
            border_thickness,
            rect_color
        );
    }

    if (!label.empty()) {
        DrawText(label.c_str(),
            static_cast<int>(position_x) + 5,
            static_cast<int>(position_y) + 5,
            16, WHITE);
    }

    Vector2 mouse = get_mouse_position();
    DrawCircleV(mouse, 5, YELLOW);

    char info[256];
    snprintf(info, sizeof(info),
        "Mouse: %.0f, %.0f | Pos: %.0f, %.0f | Size: %.0f x %.0f",
        mouse.x, mouse.y, position_x, position_y, width, height);
    DrawText(info, 10, 10, 16, WHITE);

    DrawText("WASD/Arrows: Move | Scroll: Resize | Space: Hide", 10, 30, 14, LIGHTGRAY);
    DrawText("F: Follow mouse | G: Toggle fill | R: Reset | 1-4: Colors", 10, 46, 14, LIGHTGRAY);
    DrawText("Shift: Fast | Ctrl: Slow | LMB/RMB/MMB: Change color", 10, 62, 14, LIGHTGRAY);

    if (follow_mouse) {
        DrawText("MODE: Following Mouse", 10, 82, 14, GREEN);
    }
}
