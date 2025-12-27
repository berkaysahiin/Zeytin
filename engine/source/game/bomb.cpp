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
    
    // Returns 0-1, where 1 is most dangerous (time almost up)
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
    
    // Pulse based on danger level - faster when more dangerous
    float pulse_freq = pulse_speed * (1.0f + danger * 2.0f);
    float pulse = 0.5f + 0.5f * sin(m_timer * pulse_freq);
    
    // Glow gets stronger as danger increases
    float glow_alpha = (100.0f + danger * 155.0f) * pulse;
    float scaled_glow = glow_size * scale.x * (1.0f + pulse * pulse_intensity);
    
    // Multiple layers for smooth glow
    for (int i = 5; i > 0; i--) {
        float layer_radius = scaled_glow * (i / 5.0f);
        float layer_alpha = (glow_alpha / i) * (danger * 0.5f + 0.5f);
        
        Color glow_color = {255, 0, 0, (unsigned char)layer_alpha};
        draw_circle(position.x, position.y, layer_radius, glow_color);
    }
}

void Bomb::draw_danger_rings() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float danger = get_danger_level();
    
    // Only show rings when danger is high
    if (danger < 0.3f) return;
    
    // Expanding rings
    float ring_spacing = 40.0f;
    int num_rings = 3;
    
    for (int i = 0; i < num_rings; i++) {
        float ring_offset = fmod(m_ring_timer + i * ring_spacing, ring_spacing * num_rings);
        float ring_radius = ring_offset * scale.x;
        
        // Fade out as ring expands
        float alpha = (1.0f - (ring_offset / (ring_spacing * num_rings))) * 200.0f * danger;
        
        Color ring_color = {255, 0, 0, (unsigned char)alpha};
        draw_circle_lines(position.x, position.y, ring_radius, ring_color);
    }
}

void Bomb::check_player_in_range() {
    auto& bomb_pos = Query::get<Position>(this);
    auto player_opt = Query::try_find_first<Player>();
    
    if (!player_opt) {
        m_player_in_range = false;
        return;
    }
    
    auto& player_pos = Query::get<Position>(&player_opt->get());
    
    float dx = player_pos.x - bomb_pos.x;
    float dy = player_pos.y - bomb_pos.y;
    float distance = sqrt(dx * dx + dy * dy);
    
    m_player_in_range = distance <= defuse_radius;
}

void Bomb::update_defuse() {
    if (m_player_in_range) {
        auto player_opt = Query::try_find_first<Player>();
        if (!player_opt || !player_opt->get().has_diffuser()) {
            m_player_missing_diffuser = true;
            return;
        }
        
        m_player_missing_diffuser = false;
        m_defuse_progress += get_frame_time() / defuse_time;
        
        if (m_defuse_progress >= 1.0f) {
            m_defuse_progress = 1.0f;
            m_is_defused = true;
            
            auto countdown_opt = Query::try_find_first<Countdown>();
            if (countdown_opt) {
                auto& countdown = countdown_opt->get();
                countdown.countdown_active = false;
            }
            
            auto end_game_opt = Query::try_find_first<EndGame>();
            if (end_game_opt) {
                end_game_opt->get().trigger_win();
            }
        }
    } else {
        m_player_missing_diffuser = false;
        m_defuse_progress -= get_frame_time() / (defuse_time * 2.0f);
        if (m_defuse_progress < 0.0f) {
            m_defuse_progress = 0.0f;
        }
    }
}

void Bomb::draw_bomb() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float danger = get_danger_level();
    
    // Pulse the bomb size based on danger
    float pulse = sin(m_timer * pulse_speed * (1.0f + danger));
    float size_scale = 2 + pulse * pulse_intensity * (0.5f + danger * 0.5f);
    float scaled_size = bomb_size * scale.x * size_scale;
    
    // Keep bomb dark/neutral - not red (so it doesn't look like enemy)
    Color bomb_body_color = {40, 40, 45, 255};
    
    // Draw bomb body
    draw_rectangle(
        position.x - scaled_size / 2,
        position.y - scaled_size / 2,
        scaled_size,
        scaled_size,
        bomb_body_color
    );
    
    // Yellow/orange outline to indicate interactable
    Color outline_color = {255, 200, 0, 255};  // Gold/yellow - universal "interact" color
    
    draw_rectangle_lines(
        position.x - scaled_size / 2,
        position.y - scaled_size / 2,
        scaled_size,
        scaled_size,
        outline_color
    );
    
    // "DEFUSE" label above bomb - always visible
    if (!m_is_defused) {
        const char* label = "DEFUSE";
        int label_width = MeasureText(label, label_size);
        
        // Pulse to draw attention
        float label_pulse = 0.7f + 0.3f * sin(m_timer * 3.0f);
        Color label_color = {255, 255, 255, (unsigned char)(255 * label_pulse)};
        
        draw_text(label,
                position.x - label_width / 2,
                position.y - scaled_size / 2 - 30,
                label_size,
                label_color);
    }
    
    // Warning text when player is close but missing diffuser
    if (m_player_missing_diffuser) {
        const char* warning = "NEED DIFFUSER!";
        int font_size = 24;
        int text_width = MeasureText(warning, font_size);
        
        // Pulsing warning
        float text_pulse = 0.7f + 0.3f * sin(m_timer * 5.0f);
        Color warning_color = {255, 100, 100, (unsigned char)(255 * text_pulse)};
        
        draw_text(warning,
                position.x - text_width / 2,
                position.y + scaled_size / 2 + 40,
                font_size,
                warning_color);
    }
    
    // Center timer display - shows urgency
    if (!m_is_defused) {
        auto countdown_opt = Query::try_find_first<Countdown>();
        if (countdown_opt) {
            float time_left = countdown_opt->get().get_time_remaining();
            int seconds = (int)time_left;
            
            char time_text[10];
            sprintf(time_text, "%d", seconds);
            
            int font_size = 24;
            int text_width = MeasureText(time_text, font_size);
            
            // Color based on urgency
            Color time_color;
            if (danger > 0.7f) {
                // Critical - flashing red
                float flash = 0.5f + 0.5f * sin(m_timer * 8.0f);
                time_color = {255, 0, 0, (unsigned char)(255 * flash)};
            } else if (danger > 0.4f) {
                // Warning - orange
                time_color = {255, 150, 0, 255};
            } else {
                // Normal - white
                time_color = {255, 255, 255, 255};
            }
            
            draw_text(time_text,
                    position.x - text_width / 2,
                    position.y - font_size / 2,
                    font_size,
                    time_color);
        }
    } else {
        // Green checkmark when defused
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
    
    // Pulsing circle to invite player in
    float pulse = 0.7f + 0.3f * sin(m_timer * 2.0f);
    
    if (m_player_in_range) {
        // Green when player is inside - "you're doing it right!"
        Color active_color = {0, 255, 0, (unsigned char)(150 * pulse)};
        draw_circle_lines(position.x, position.y, scaled_radius, active_color);
        draw_circle(position.x, position.y, scaled_radius, ColorAlpha(active_color, 0.2f));
    } else {
        // Yellow/orange when not in range - "come here!"
        Color inactive_color = {255, 200, 0, (unsigned char)(100 * pulse)};
        draw_circle_lines(position.x, position.y, scaled_radius, inactive_color);
        draw_circle(position.x, position.y, scaled_radius, ColorAlpha(inactive_color, 0.1f));
        
        // Add arrow/indicator pointing to circle edge
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
    
    draw_rectangle(bar_x, bar_y, scaled_width, scaled_height, bar_bg_color);
    
    float progress_width = scaled_width * m_defuse_progress;
    draw_rectangle(bar_x, bar_y, progress_width, scaled_height, get_bar_color());
    
    draw_rectangle_lines(bar_x, bar_y, scaled_width, scaled_height, WHITE);
    
    char progress_text[10];
    sprintf(progress_text, "%d%%", (int)(m_defuse_progress * 100));
    int font_size = 12;
    int text_width = MeasureText(progress_text, font_size);
    draw_text(progress_text,
            position.x - text_width / 2,
            bar_y + scaled_height + 5,
            font_size,
            WHITE);
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
