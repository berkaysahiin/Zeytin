#include "game/retry.h"
#include "core/zeytin.h"
#include "core/raylib_wrapper.h"

void Retry::on_update() {
    draw_retry_button();
}

void Retry::on_play_update() {
    if (is_key_pressed(KEY_R)) {
        restart_game();
    }

    if (is_button_clicked()) {
        restart_game();
    }
}

void Retry::draw_retry_button() {
    Rectangle button_rect = {
        button_x,
        button_y,
        button_width,
        button_height
    };
    
    Vector2 mouse_pos = get_mouse_position();
    m_is_hovered = CheckCollisionPointRec(mouse_pos, button_rect);
    
    Color current_color = m_is_hovered ? button_hover_color : button_color;
    draw_rectangle_rec(button_rect, current_color);
    
    DrawRectangleLinesEx(button_rect, 2.0f, text_color);
    
    const char* button_text = "RETRY (R)";
    int text_width = MeasureText(button_text, button_font_size);
    
    float text_x = button_x + (button_width - text_width) / 2.0f;
    float text_y = button_y + (button_height - button_font_size) / 2.0f;
    
    draw_text(button_text, text_x, text_y, button_font_size, text_color);
}

bool Retry::is_button_clicked() {
    if (!m_is_hovered) return false;
    
    return is_mouse_button_pressed(MOUSE_LEFT_BUTTON);
}

void Retry::restart_game() {
    Zeytin::get().reload_scene();
}
