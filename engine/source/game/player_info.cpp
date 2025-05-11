#include "game/player_info.h"
#include "core/raylib_wrapper.h"
#include "core/query.h"
#include "core/zeytin.h"
#include "raylib.h"
#include "game/scale.h"
#include "game/position.h"
#include "game/collider.h"

void PlayerInfo::on_post_init() {
    color = get_random_color();
}

void PlayerInfo::on_play_update() {
    if (in_zone) {
        since_last_zone = 0;
    } else {
        since_last_zone += get_frame_time();
        
        if (since_last_zone >= max_time_outside) {
            time_spent_zone = fmaxf(0.0f, time_spent_zone - get_frame_time() * 2.0f);
        }
    }
    
    draw_ui();
    draw_zone_warning();
}

void PlayerInfo::draw_ui() {
    const float screen_width = VIRTUAL_WIDTH;
    const float screen_height = VIRTUAL_HEIGHT;

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

    if (!in_zone && since_last_zone > 3.0f) {
        const float remaining_time = max_time_outside - since_last_zone;
        
        if (remaining_time > 0) {
            char tolerance_text[32];
            sprintf(tolerance_text, "ZONE TIMEOUT: %.1fs", remaining_time);
            
            Color tolerance_color;
            if (remaining_time < 3.0f) {
                float flash = sinf(get_time() * 10.0f) * 0.5f + 0.5f;
                tolerance_color = ColorAlpha(RED, 0.5f + flash * 0.5f);
            } else if (remaining_time < 6.0f) {
                tolerance_color = RED;
            } else {
                tolerance_color = ORANGE;
            }
            
            DrawText(tolerance_text, panel_x + m_padding, m_panel_y + m_padding + 55, 18, tolerance_color);
        } else {
            DrawText("ZONE PENALTY ACTIVE!", panel_x + m_padding, m_panel_y + m_padding + 55, 18, RED);
        }
        
        char time_text[32];
        sprintf(time_text, "%.1f / 100s", time_spent_zone);
        DrawText(time_text, panel_x + m_padding, m_panel_y + m_padding + 80, 18, WHITE);
        
        float bar_y = m_panel_y + m_padding + 105;
        float bar_width = m_panel_width - m_padding * 2;
        Rectangle bar_bg = {panel_x + m_padding, bar_y, bar_width, 16};
        float max_zone_time = 100.0f;
        float fill_ratio = Clamp(time_spent_zone / max_zone_time, 0.0f, 1.0f);
        Rectangle bar_fill = {panel_x + m_padding, bar_y, bar_width * fill_ratio, 16};
        
        DrawRectangleRec(bar_bg, DARKGRAY);
        DrawRectangleRec(bar_fill, zone_color);
    } else {
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
    }

    if (!in_zone) {
        float pulse = (sinf((float)get_time() * 5.0f) * 0.5f + 0.5f);
        float outline_thickness = 2.0f + pulse * 3.0f;
        DrawRectangleLinesEx(panel_rect, outline_thickness,
            ColorAlpha(RED, 0.5f + pulse * 0.5f));
    }
}

void PlayerInfo::draw_zone_warning() {
    if (in_zone || since_last_zone < warning_threshold) {
        return;
    }
    
    const float remaining_time = max_time_outside - since_last_zone;
    
    if (remaining_time <= 0) {
        return;
    }
    
    auto pos = Query::read<Position>(this);
    auto scale = Query::try_get<Scale>(this);
    auto collider = Query::try_get<Collider>(this);
    
    float y_offset = 0;
    if (collider) {
        y_offset = collider->get().m_height + 0.75f;
        if (scale) {
            y_offset *= scale->get().y;
        }
    } else {
        y_offset = 60.0f; 
    }

    Color warning_color;
    const char* warning_text;
    int font_size = 20;
    
    if (since_last_zone > critical_threshold) {
        float flash = sinf(get_time() * 15.0f) * 0.5f + 0.5f;
        warning_color = ColorAlpha(RED, 0.5f + flash * 0.5f);
        warning_text = "RETURN TO ZONE!";
        font_size = 22;
    } else if (since_last_zone > danger_threshold) {
        warning_color = RED;
        warning_text = "FIND ZONE!";
        font_size = 20;
    } else if (since_last_zone > warning_threshold) {
        warning_color = ORANGE;
        warning_text = "ZONE REQUIRED";
    } else {
        return; 
    }
    
    char time_text[20];
    sprintf(time_text, "%.1fs", remaining_time);
    
    int warning_width = MeasureText(warning_text, font_size);
    int time_width = MeasureText(time_text, font_size);
    
    draw_text(warning_text, 
              pos.x - warning_width/2 + 2, 
              pos.y - y_offset + 2,
              font_size, 
              BLACK);
    
    draw_text(warning_text, 
              pos.x - warning_width/2, 
              pos.y - y_offset,
              font_size, 
              warning_color);
    
    draw_text(time_text,
              pos.x - time_width/2 + 2,
              pos.y - y_offset + font_size + 2,
              font_size,
              BLACK);
    
    draw_text(time_text,
              pos.x - time_width/2,
              pos.y - y_offset + font_size,
              font_size,
              warning_color);
}
