#include "game/bomb.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/position.h"
#include "game/scale.h"
#include "game/player.h"
#include "game/countdown.h"
#include "game/end_game.h"

void Bomb::on_init() {
    m_is_defused = false;
    m_defuse_progress = 0.0f;
    m_player_in_range = false;
}

void Bomb::on_update() {
    draw_bomb();
    draw_defuse_circle();
    draw_defuse_bar();
}

void Bomb::on_play_update() {
    if (m_is_defused) return;
    
    check_player_in_range();
    update_defuse();
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
            return;
        }
        
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
        m_defuse_progress -= get_frame_time() / (defuse_time * 2.0f);
        if (m_defuse_progress < 0.0f) {
            m_defuse_progress = 0.0f;
        }
    }
}

void Bomb::draw_bomb() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float scaled_size = bomb_size * scale.x;
    
    draw_rectangle(
        position.x - scaled_size / 2,
        position.y - scaled_size / 2,
        scaled_size,
        scaled_size,
        bomb_color
    );
    
    // Draw bomb outline
    draw_rectangle_lines(
        position.x - scaled_size / 2,
        position.y - scaled_size / 2,
        scaled_size,
        scaled_size,
        {200, 200, 200, 255}
    );
    
    if (!m_is_defused) {
        const char* warning = "!";
        int font_size = 30;
        int text_width = MeasureText(warning, font_size);
        draw_text(warning,
                position.x - text_width / 2,
                position.y - font_size / 2,
                font_size,
                {255, 0, 0, 255});
    } else {
        // Draw checkmark when defused
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
    
    Color circle_col = m_player_in_range ? 
        Color{0, 255, 0, 100} : circle_color;
    
    draw_circle_lines(position.x, position.y, scaled_radius, circle_col);
    
    draw_circle(position.x, position.y, scaled_radius, 
               ColorAlpha(circle_col, 0.1f));
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
