#include "game/bomb.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/position.h"
#include "game/scale.h"
#include "game/player.h"
#include "game/countdown.h"
#include "game/end_game.h"
#include <cmath>

void Bomb::on_init() {
    m_is_defused = false;
    m_defuse_progress = 0.0f;
    m_player_in_range = false;
    m_player_missing_diffuser = false;
    m_timer = 0.0f;
    m_ring_timer = 0.0f;
}

void Bomb::on_update() {
    m_timer += get_frame_time();
    m_ring_timer += get_frame_time() * ring_speed;
    
    if (!m_is_defused) {
        draw_danger_effects();
    }
    
    draw_bomb();
    draw_defuse_circle();
    draw_defuse_bar();
}

void Bomb::on_play_update() {
    if (m_is_defused) return;
    
    check_player_in_range();
    update_defuse();
}

float Bomb::get_danger_level() const {
    auto countdown_opt = Query::try_find_first<Countdown>();
    if (!countdown_opt) return 0.0f;
    
    float time_remaining = countdown_opt->get().get_time_remaining();
    float total_time = countdown_opt->get().duration;
    
    return 1.0f - (time_remaining / total_time);
}

void Bomb::draw_danger_effects() {
    draw_red_glow();
    
    if (show_danger_rings) {
        draw_danger_rings();
    }
}

void Bomb::draw_red_glow() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float danger = get_danger_level();
    
    float pulse_freq = pulse_speed * (1.0f + danger * 2.0f);
    float glow_pulse = 0.3f + 0.7f * abs(sin(m_timer * pulse_freq));
    
    float scaled_glow = glow_size * scale.x * (0.8f + danger * 0.4f);
    
    unsigned char alpha = (unsigned char)(80 * glow_pulse * danger);
    Color glow_color = {255, 0, 0, alpha};
    
    draw_circle(position.x, position.y, scaled_glow, glow_color);
}

void Bomb::draw_danger_rings() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float danger = get_danger_level();
    if (danger < 0.3f) return;
    
    int num_rings = 3;
    for (int i = 0; i < num_rings; i++) {
        float ring_offset = (float)i / num_rings;
        float ring_phase = fmod(m_ring_timer + ring_offset, 1.0f);
        
        float ring_radius = ring_phase * defuse_radius * scale.x * 1.5f;
        float ring_alpha = (1.0f - ring_phase) * danger;
        
        Color ring_color = {255, 100, 0, (unsigned char)(100 * ring_alpha)};
        draw_circle_lines(position.x, position.y, ring_radius, ring_color);
    }
}

void Bomb::check_player_in_range() {
    auto player_opt = Query::try_find_first<Player>();
    if (!player_opt) {
        m_player_in_range = false;
        m_player_missing_diffuser = false;
        return;
    }
    
    auto& player = player_opt->get();
    auto& player_pos = Query::get<Position>(&player);
    auto& bomb_pos = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float dx = player_pos.x - bomb_pos.x;
    float dy = player_pos.y - bomb_pos.y;
    float distance = sqrt(dx * dx + dy * dy);
    
    float scaled_radius = defuse_radius * scale.x;
    
    bool in_range = distance <= scaled_radius;
    bool has_diffuser = player.has_diffuser();
    
    m_player_in_range = in_range && has_diffuser;
    m_player_missing_diffuser = in_range && !has_diffuser;
	m_player_defusing = m_player_in_range && !m_player_missing_diffuser;
}

void Bomb::update_defuse() {
    if (!m_player_in_range) {
        m_defuse_progress = 0.0f;
        return;
    }
    
    m_defuse_progress += get_frame_time() / defuse_time;
    
    if (m_defuse_progress >= 1.0f) {
        m_is_defused = true;
        m_defuse_progress = 1.0f;
        
        //auto countdown_opt = Query::try_find_first<Countdown>();
        //if (countdown_opt) {
        //    countdown_opt->get().stop();
        //}
        
        auto end_game_opt = Query::try_find_first<EndGame>();
        if (end_game_opt) {
            end_game_opt->get().trigger_win();
        }
    }
}

void Bomb::draw_bomb() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    if (m_is_defused) {
        draw_rectangle(
            position.x - bomb_size * scale.x / 2,
            position.y - bomb_size * scale.y / 2,
            bomb_size * scale.x,
            bomb_size * scale.y,
            {50, 200, 50, 255}
        );
        return;
    }
    
    float danger = get_danger_level();
    
    float pulse = sin(m_timer * pulse_speed * (1.0f + danger));
    float size_scale = 2 + pulse * pulse_intensity * (0.5f + danger * 0.5f);
    float scaled_size = bomb_size * scale.x * size_scale;
    
    Color bomb_body_color = {40, 40, 45, 255};
    
    draw_rectangle(
        position.x - scaled_size / 2,
        position.y - scaled_size / 2,
        scaled_size,
        scaled_size,
        bomb_body_color
    );
    
    Color outline_color = {255, 200, 0, 255};
    
    draw_rectangle_lines(
        position.x - scaled_size / 2,
        position.y - scaled_size / 2,
        scaled_size,
        scaled_size,
        outline_color
    );
    
    if (!m_is_defused) {
        const char* label = "DEFUSE";
        int label_width = MeasureText(label, label_size);
        
        float label_pulse = 0.7f + 0.3f * sin(m_timer * 3.0f);
        Color label_color = {255, 255, 255, (unsigned char)(255 * label_pulse)};
        
        draw_text(label,
                position.x - label_width / 2,
                position.y - scaled_size / 2 - 30,
                label_size,
                label_color);
    }
    
    if (m_player_missing_diffuser) {
        const char* warning = "NEED DIFFUSER!";
        int font_size = 24;
        int text_width = MeasureText(warning, font_size);
        
        float text_pulse = 0.7f + 0.3f * sin(m_timer * 5.0f);
        Color warning_color = {255, 100, 100, (unsigned char)(255 * text_pulse)};
        
        draw_text(warning,
                position.x - text_width / 2,
                position.y + scaled_size / 2 + 40,
                font_size,
                warning_color);
    }
    
    if (!m_is_defused) {
        auto countdown_opt = Query::try_find_first<Countdown>();
        if (countdown_opt) {
            float time_left = countdown_opt->get().get_time_remaining();
            int seconds = (int)time_left;
            
            char time_text[10];
            sprintf(time_text, "%d", seconds);
            
            int font_size = 40;
            int text_width = MeasureText(time_text, font_size);
            
            Color time_color;
            if (danger > 0.7f) {
                float flash = 0.5f + 0.5f * sin(m_timer * 8.0f);
                time_color = {255, 0, 0, (unsigned char)(255 * flash)};
            } else if (danger > 0.4f) {
                time_color = {255, 150, 0, 255};
            } else {
                time_color = {255, 255, 255, 255};
            }
            
            draw_text(time_text,
                    position.x - text_width / 2,
                    position.y - font_size / 2,
                    font_size,
                    time_color);
        }
    } else {
        const char* checkmark = "✓";
        int font_size = 30;
        int text_width = MeasureText(checkmark, font_size);
        draw_text(checkmark,
                position.x - text_width / 2,
                position.y - font_size / 2,
                font_size,
                {0, 255, 0, 255});
    }
}

void Bomb::draw_defuse_circle() {
    if (m_is_defused) return;
    
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float scaled_radius = defuse_radius * scale.x;
    
    float pulse = 0.7f + 0.3f * sin(m_timer * 2.0f);
    
    if (m_player_in_range) {
        Color active_color = {0, 255, 0, (unsigned char)(150 * pulse)};
        draw_circle_lines(position.x, position.y, scaled_radius, active_color);
        draw_circle(position.x, position.y, scaled_radius, ColorAlpha(active_color, 0.2f));
    } else {
        Color inactive_color = {255, 200, 0, (unsigned char)(100 * pulse)};
        draw_circle_lines(position.x, position.y, scaled_radius, inactive_color);
        draw_circle(position.x, position.y, scaled_radius, ColorAlpha(inactive_color, 0.1f));
        
        const char* hint = "ENTER CIRCLE";
        int hint_size = 16;
        int hint_width = MeasureText(hint, hint_size);
        
        float hint_pulse = 0.6f + 0.4f * sin(m_timer * 3.0f);
        Color hint_color = {255, 255, 255, (unsigned char)(200 * hint_pulse)};
        
        draw_text(hint,
                position.x - hint_width / 2,
                position.y + scaled_radius + 10,
                hint_size,
                hint_color);
    }
}

void Bomb::draw_defuse_bar() {
    if (m_defuse_progress <= 0.0f) return;
    
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float scaled_width = bar_width * scale.x;
    float scaled_height = bar_height * scale.y;
    float scaled_offset_y = bar_offset_y * scale.y;
    
    float bar_x = position.x - scaled_width / 2;
    float bar_y = position.y + scaled_offset_y;
    
    // Black background border
    float border = bar_border_thickness;
    draw_rectangle(bar_x - border, bar_y - border, scaled_width + border * 2, scaled_height + border * 2, {0, 0, 0, 255});
    
    // Dark gray background
    draw_rectangle(bar_x, bar_y, scaled_width, scaled_height, {30, 30, 30, 255});
    
    // Bright progress bar
    float progress_width = scaled_width * m_defuse_progress;
    draw_rectangle(bar_x, bar_y, progress_width, scaled_height, get_bar_color());
    
    // White outline
    float outline = bar_outline_thickness;
    draw_rectangle_lines(bar_x - outline, bar_y - outline, scaled_width + outline * 2, scaled_height + outline * 2, WHITE);
    draw_rectangle_lines(bar_x, bar_y, scaled_width, scaled_height, WHITE);
    
    char progress_text[10];
    sprintf(progress_text, "%d%%", (int)(m_defuse_progress * 100));
    int text_width = MeasureText(progress_text, bar_progress_font_size);
    
    float text_y = bar_y + scaled_height + bar_text_offset_y;
    
    // Text with black shadow
   // draw_text(progress_text,
   //         position.x - text_width / 2 + bar_text_shadow_offset,
   //         text_y + bar_text_shadow_offset,
   //         bar_progress_font_size,
   //         BLACK);
   // draw_text(progress_text,
   //         position.x - text_width / 2,
   //         text_y,
   //         bar_progress_font_size,
   //         WHITE);
}

Color Bomb::get_bar_color() const {
    float r = 255.0f * (1.0f - m_defuse_progress);
    float g = 255.0f * m_defuse_progress;
    
    return Color{
        (unsigned char)r,
        (unsigned char)g,
        0,
        255
    };
}
