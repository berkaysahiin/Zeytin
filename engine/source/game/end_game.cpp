#include "game/end_game.h"
#include "core/raylib_wrapper.h"
#include "core/zeytin.h"

void EndGame::on_init() {
    m_game_over = false;
    m_is_win = false;
    m_post_game_timer = 0.0f;
    m_fade_timer = 0.0f;
}

void EndGame::on_play_update() {
    if (m_game_over) {
        m_post_game_timer += get_frame_time();
        m_fade_timer += get_frame_time();
        
        if (m_is_win) {
            draw_win_screen();
        } else {
            draw_game_over_screen();
        }
        
        if (get_keycode_pressed() != 0 && m_post_game_timer > 2.5f) {
            // On win: load next level if specified, otherwise restart
            // On lose: always restart
            if (m_is_win && !next_level.empty()) {
                Zeytin::get().request_level_load(next_level);
            } else {
                restart_game();
            }
        }
    }
}

void EndGame::trigger_game_over(const std::string& reason) {
    if (m_game_over) return;
    
    m_game_over = true;
    m_is_win = false;
    m_post_game_timer = 0.0f;
    m_fade_timer = 0.0f;
    m_lose_reason = reason;
}

void EndGame::trigger_win() {
    if (m_game_over) return;
    
    m_game_over = true;
    m_is_win = true;
    m_post_game_timer = 0.0f;
    m_fade_timer = 0.0f;
    m_lose_reason = "Bomb defused!";
}

void EndGame::restart_game() {
    Zeytin::get().reload_scene();
}

void EndGame::draw_game_over_screen() {
    float screen_width = VIRTUAL_WIDTH;
    float screen_height = VIRTUAL_HEIGHT;
    
    float alpha = std::min(m_fade_timer / 0.5f, 1.0f) * ui_transparency;
    draw_rectangle(0, 0, screen_width, screen_height, ColorAlpha(BLACK, alpha));
    
    if (m_fade_timer >= 0.3f) {
        float text_alpha = std::min((m_fade_timer - 0.3f) / 0.5f, 1.0f);
        
        const char* game_over_text = "GAME OVER";
        int text_width = MeasureText(game_over_text, game_over_font_size);
        
        draw_text(game_over_text,
                (screen_width - text_width) / 2 + 4,
                screen_height * 0.35f + 4,
                game_over_font_size,
                ColorAlpha(BLACK, text_alpha * 0.7f));
        
        float pulse = 0.9f + 0.1f * sinf(m_fade_timer * 3.0f);
        draw_text(game_over_text,
                (screen_width - text_width) / 2,
                screen_height * 0.35f,
                game_over_font_size,
                ColorAlpha(RED, text_alpha * pulse));
        
        if (!m_lose_reason.empty() && m_fade_timer >= 0.7f) {
            float reason_alpha = std::min((m_fade_timer - 0.7f) / 0.5f, 1.0f);
            int reason_width = MeasureText(m_lose_reason.c_str(), reason_font_size);
            
            draw_text(m_lose_reason.c_str(),
                    (screen_width - reason_width) / 2,
                    screen_height * 0.5f,
                    reason_font_size,
                    ColorAlpha(ORANGE, reason_alpha));
        }
        
        if (m_fade_timer >= 2.0f) {
            float prompt_alpha = std::min((m_fade_timer - 2.0f) / 0.5f, 1.0f);
            const char* continue_text = "Press any key to restart";
            int continue_width = MeasureText(continue_text, reason_font_size * 0.8f);
            
            float blink = (sinf(m_fade_timer * 5.0f) * 0.5f + 0.5f) * prompt_alpha;
            draw_text(continue_text,
                    (screen_width - continue_width) / 2,
                    screen_height * 0.7f,
                    reason_font_size * 0.8f,
                    ColorAlpha(WHITE, blink));
        }
    }
}

void EndGame::draw_win_screen() {
    float screen_width = VIRTUAL_WIDTH;
    float screen_height = VIRTUAL_HEIGHT;
    
    float alpha = std::min(m_fade_timer / 0.5f, 1.0f) * ui_transparency;
    draw_rectangle(0, 0, screen_width, screen_height, 
                  ColorAlpha({20, 40, 20, 255}, alpha));
    
    if (m_fade_timer >= 0.3f) {
        float text_alpha = std::min((m_fade_timer - 0.3f) / 0.5f, 1.0f);
        
        const char* win_text = "LEVEL COMPLETE!";
        int text_width = MeasureText(win_text, win_font_size);
        
        draw_text(win_text,
                (screen_width - text_width) / 2 + 4,
                screen_height * 0.35f + 4,
                win_font_size,
                ColorAlpha(BLACK, text_alpha * 0.7f));
        
        float pulse = 0.9f + 0.1f * sinf(m_fade_timer * 3.0f);
        draw_text(win_text,
                (screen_width - text_width) / 2,
                screen_height * 0.35f,
                win_font_size,
                ColorAlpha(GREEN, text_alpha * pulse));
        
        if (m_fade_timer >= 0.7f) {
            float reason_alpha = std::min((m_fade_timer - 0.7f) / 0.5f, 1.0f);
            int reason_width = MeasureText(m_lose_reason.c_str(), reason_font_size);
            
            draw_text(m_lose_reason.c_str(),
                    (screen_width - reason_width) / 2,
                    screen_height * 0.5f,
                    reason_font_size,
                    ColorAlpha(GOLD, reason_alpha));
        }
        
        if (m_fade_timer >= 2.0f) {
            float prompt_alpha = std::min((m_fade_timer - 2.0f) / 0.5f, 1.0f);
            
            // Different text based on whether we have a next level
            const char* continue_text = next_level.empty() 
                ? "Press any key to restart"
                : "Press any key to continue";
            
            int continue_width = MeasureText(continue_text, reason_font_size * 0.8f);
            
            float blink = (sinf(m_fade_timer * 5.0f) * 0.5f + 0.5f) * prompt_alpha;
            draw_text(continue_text,
                    (screen_width - continue_width) / 2,
                    screen_height * 0.7f,
                    reason_font_size * 0.8f,
                    ColorAlpha(WHITE, blink));
        }
    }
}

int EndGame::get_keycode_pressed() {
    const int keycodes[] = {
        KEY_SPACE, KEY_ENTER, KEY_ESCAPE, 
        KEY_A, KEY_S, KEY_D, KEY_W,
        KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
    };
    
    for (int key : keycodes) {
        if (is_key_pressed(key)) {
            return key;
        }
    }
    
    for (int i = 0; i < 3; i++) {
        if (is_mouse_button_pressed(i)) {
            return 1000 + i; 
        }
    }
    
    return 0; 
}
