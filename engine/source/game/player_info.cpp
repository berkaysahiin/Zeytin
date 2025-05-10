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

    float panel_x = (index == 0) ? screen_width - m_panel_width - m_panel_x : m_panel_x;
    Rectangle panel_rect = {panel_x, m_panel_y, m_panel_width, m_panel_height};

    Color bg_color = {10, 10, 30, 230};
    Color zone_color = in_zone ? GREEN : RED;

    DrawRectangleRounded(panel_rect, 0.15f, 8, bg_color);

    char player_title[32];
    const char* controls = (index == 0) ? "ARROWS" : "WASD";
    sprintf(player_title, "PLAYER %d (%s)", index + 1, controls);
    DrawText(player_title, panel_x + m_padding, m_panel_y + m_padding, 20, WHITE);

    const char* zone_status = in_zone ? "IN ZONE" : "OUTSIDE";
    DrawText(zone_status, panel_x + m_padding, m_panel_y + m_padding + 30, 18, zone_color);

    char time_text[32];
    sprintf(time_text, "%.1f / 100s", time_spent_zone);
    DrawText(time_text, panel_x + m_padding, m_panel_y + m_padding + 55, 18, WHITE);

    float max_zone_time = 100.0f;
    float fill_ratio = Clamp(time_spent_zone / max_zone_time, 0.0f, 1.0f);

    float bar_y = m_panel_y + m_padding + 80;
    float bar_width = m_panel_width - m_padding * 2;
    Rectangle bar_bg = {panel_x + m_padding, bar_y, bar_width, 16};
    Rectangle bar_fill = {panel_x + m_padding, bar_y, bar_width * fill_ratio, 16};

    DrawRectangleRec(bar_bg, DARKGRAY);
    DrawRectangleRec(bar_fill, zone_color);

    if (!in_zone) {
        float pulse = (sinf((float)get_time() * 5.0f) * 0.5f + 0.5f);
        float outline_thickness = 2.0f + pulse * 3.0f;
        DrawRectangleLinesEx(panel_rect, outline_thickness,
            ColorAlpha(RED, 0.5f + pulse * 0.5f));
    }
}

