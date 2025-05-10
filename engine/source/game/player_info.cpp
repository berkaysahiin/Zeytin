#include "game/player_info.h"
#include "core/raylib_wrapper.h"
#include "raylib.h"

void PlayerInfo::on_post_init() {
    color = get_random_color();
}

void PlayerInfo::on_play_update() {
    draw_ui();
}

void PlayerInfo::draw_ui() {
    const float screen_width = get_screen_width();
    const float screen_height = get_screen_height();

    float panel_x = 0.0f;
    if (index == 0) {
        panel_x = screen_width - m_panel_width - 20.0f;
    } else {
        panel_x = m_panel_x;
    }

    Rectangle panel_rect = {panel_x, m_panel_y, m_panel_width, m_panel_height};
    Color bg_color = {10, 10, 30, 230}; 
    Color border_color = color; 

    DrawRectangleRounded(panel_rect, 0.15f, 8, bg_color);

    char player_title[32];
    const char* controls = (index == 0) ? "ARROWS" : "WASD";
    sprintf(player_title, "PLAYER %d (%s)", index + 1, controls);

    DrawText(player_title, panel_x + m_padding + 5, m_panel_y + m_padding, 24, WHITE);

    const char* zone_status = in_zone ? "IN ZONE" : "OUTSIDE";
    Color zone_color = in_zone ? GREEN : RED;

    DrawText("STATUS:", panel_x + m_padding, m_panel_y + m_padding + 30, 20, WHITE);
    DrawText(zone_status, panel_x + m_padding + 90, m_panel_y + m_padding + 30, 20, zone_color);

    char time_text[32];
    sprintf(time_text, "ZONE TIME: %.1f s", time_spent_zone);
    DrawText(time_text, panel_x + m_padding, m_panel_y + m_padding + 90, 20, WHITE);

    Rectangle bar_bg = {panel_x + m_padding, m_panel_y + m_padding + 120, m_panel_width - m_padding*2, 20};

    float max_zone_time = 10.0f;
    float fill_ratio = 1.0f - (time_spent_zone / max_zone_time);
    fill_ratio = (fill_ratio < 0.0f) ? 0.0f : (fill_ratio > 1.0f) ? 1.0f : fill_ratio;

    Rectangle bar_fill = {panel_x + m_padding, m_panel_y + m_padding + 120,
                         (m_panel_width - m_padding*2) * fill_ratio, 20};

    DrawRectangleRec(bar_bg, DARKGRAY);
    DrawRectangleRec(bar_fill, zone_color);

    DrawText("HEALTH", panel_x + m_padding + 5, m_panel_y + m_padding + 123, 14, BLACK);

    if (!in_zone) {
        float pulse = (sinf((float)get_time() * 5.0f) * 0.5f + 0.5f);
        float outline_thickness = 2.0f + pulse * 3.0f;

        DrawRectangleLinesEx(panel_rect, outline_thickness,
                            ColorAlpha(RED, 0.5f + pulse * 0.5f));
    }
}
