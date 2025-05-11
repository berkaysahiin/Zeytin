#include "game/end_game.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "core/zeytin.h"
#include "game/particle_system.h"
#include "game/player_info.h"
#include "game/position.h"
#include "game/start_game.h"
#include "remote_logger/remote_logger.h"

#include "core/embeded_scene.h"

void EndGame::on_init() {
    m_game_over = false;
    m_post_game_timer = 0.0f;
    m_fade_timer = 0.0f;
}

void EndGame::on_play_update() {
    if (m_game_over) {
        m_post_game_timer += get_frame_time();
        m_fade_timer += get_frame_time();
        
        draw_game_over_screen();
        
        if (get_keycode_pressed() != 0 && m_post_game_timer > 1.0f) {
            restart_game();
        }
    }
}

void EndGame::mark_player_lost(int player_index, const std::string& reason) {
    if (m_game_over) return; 
    
    auto players = Query::find_where<PlayerInfo>([player_index](const PlayerInfo& p) {
        return p.index == player_index;
    });
    
    if (players.empty()) return;
    
    PlayerResult result;
    auto& player = players[0].get();
    result.index = player.index;
    result.name = player.name.empty() ? "Player " + std::to_string(player.index + 1) : player.name;
    result.score = player.time_spent_zone;
    result.color = player.color;
    result.winner = false;
    
    m_results.push_back(result);
    
    auto all_players = Query::find_all<PlayerInfo>();
    for (auto& p_ref : all_players) {
        auto& p = p_ref.get();
        if (p.index != player_index) {
            PlayerResult winner_result;
            winner_result.index = p.index;
            winner_result.name = p.name.empty() ? "Player " + std::to_string(p.index + 1) : p.name;
            winner_result.score = p.time_spent_zone;
            winner_result.color = p.color;
            winner_result.winner = true;
            
            m_results.push_back(winner_result);
        }
    }
    
    m_game_over = true;
    m_post_game_timer = 0.0f;
    m_fade_timer = 0.0f;
    m_lose_reason = reason;
    
    try_spawn_winner_particles();
}

void EndGame::end_game(const std::string& reason) {
    if (m_game_over) return;
    
    m_results.clear();
    
    auto all_players = Query::find_all<PlayerInfo>();
    
    float max_score = 0.0f;
    for (auto& p_ref : all_players) {
        auto& p = p_ref.get();
        max_score = std::max(max_score, p.time_spent_zone);
    }
    
    for (auto& p_ref : all_players) {
        auto& p = p_ref.get();
        PlayerResult result;
        result.index = p.index;
        result.name = p.name.empty() ? "Player " + std::to_string(p.index + 1) : p.name;
        result.score = p.time_spent_zone;
        result.color = p.color;
        result.winner = (p.time_spent_zone == max_score);
        
        m_results.push_back(result);
    }
    
    std::sort(m_results.begin(), m_results.end(), 
        [](const PlayerResult& a, const PlayerResult& b) {
            return a.score > b.score;
        });
    
    m_game_over = true;
    m_post_game_timer = 0.0f;
    m_fade_timer = 0.0f;
    m_lose_reason = reason;
    
    try_spawn_winner_particles();
    
    auto& game_started = Query::find_first<StartGame>();
    game_started.game_started = false;
}

void EndGame::restart_game() {
    m_game_over = false;
    m_post_game_timer = 0.0f;
    m_fade_timer = 0.0f;
    m_results.clear();

    Zeytin::get().deserialize_scene(scene);
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
                screen_height * 0.25f + 4,
                game_over_font_size,
                ColorAlpha(BLACK, text_alpha * 0.7f));
        
        float pulse = 0.9f + 0.1f * sinf(m_fade_timer * 3.0f);
        draw_text(game_over_text,
                (screen_width - text_width) / 2,
                screen_height * 0.25f,
                game_over_font_size,
                ColorAlpha(WHITE, text_alpha * pulse));
        
        if (!m_lose_reason.empty() && m_fade_timer >= 0.7f) {
            float reason_alpha = std::min((m_fade_timer - 0.7f) / 0.5f, 1.0f);
            int reason_width = MeasureText(m_lose_reason.c_str(), results_font_size);
            
            draw_text(m_lose_reason.c_str(),
                    (screen_width - reason_width) / 2,
                    screen_height * 0.35f,
                    results_font_size,
                    ColorAlpha(ORANGE, reason_alpha));
        }
        
        if (m_fade_timer >= 1.0f) {
            draw_results(std::min((m_fade_timer - 1.0f) / 0.5f, 1.0f));
        }
        
        if (m_fade_timer >= 2.0f) {
            float prompt_alpha = std::min((m_fade_timer - 2.0f) / 0.5f, 1.0f);
            const char* continue_text = "Press any key to continue";
            int continue_width = MeasureText(continue_text, results_font_size * 0.7f);
            
            float blink = (sinf(m_fade_timer * 5.0f) * 0.5f + 0.5f) * prompt_alpha;
            draw_text(continue_text,
                    (screen_width - continue_width) / 2,
                    screen_height * 0.85f,
                    results_font_size * 0.7f,
                    ColorAlpha(WHITE, blink));
        }
    }
}

void EndGame::draw_results(float alpha) {
    float screen_width = VIRTUAL_WIDTH;
    float screen_height = VIRTUAL_HEIGHT;
    
    float start_y = screen_height * 0.45f;
    float line_height = results_font_size * 1.5f;
    
    const char* results_title = "RESULTS";
    int title_width = MeasureText(results_title, results_font_size * 1.2f);
    draw_text(results_title,
            (screen_width - title_width) / 2,
            start_y - line_height,
            results_font_size * 1.2f,
            ColorAlpha(WHITE, alpha));
    
    float line_start_x = screen_width * 0.3f;
    float line_end_x = screen_width * 0.7f;
    draw_line(line_start_x, start_y - line_height * 0.5f, 
             line_end_x, start_y - line_height * 0.5f,
             ColorAlpha(GRAY, alpha * 0.7f));
    
    for (size_t i = 0; i < m_results.size(); i++) {
        const PlayerResult& result = m_results[i];
        float y_pos = start_y + i * line_height;
        
        char rank_text[10];
        sprintf(rank_text, "#%zu", i + 1);
        draw_text(rank_text,
                screen_width * 0.3f - MeasureText(rank_text, results_font_size) - 10,
                y_pos,
                results_font_size,
                ColorAlpha(LIGHTGRAY, alpha));
        
        char result_text[100];
        sprintf(result_text, "%s: %.1f", 
                result.name.c_str(), 
                result.score);
        
        Color text_color = result.color;
        
        if (result.winner) {
            const char* winner_icon = "★";
            draw_text(winner_icon,
                    screen_width * 0.3f - 30,
                    y_pos,
                    results_font_size,
                    ColorAlpha(GOLD, alpha * (0.7f + 0.3f * sinf(m_fade_timer * 4.0f))));
            
            float pulse = 0.8f + 0.2f * sinf(m_fade_timer * 3.0f + i * 0.5f);
            text_color = ColorAlpha(result.color, alpha * pulse);
            
        } else {
            text_color = ColorAlpha(text_color, alpha);
        }
        
        draw_text(result_text,
                screen_width * 0.3f,
                y_pos,
                results_font_size,
                text_color);
    }
    
    if (show_countdown && m_post_game_timer <= post_game_delay) {
        char restart_text[100];
        sprintf(restart_text, "Returning to menu in %.1f seconds...", 
                post_game_delay - m_post_game_timer);
        
        int text_width = MeasureText(restart_text, results_font_size * 0.6f);
        float y_pos = screen_height * 0.75f;
        
        draw_text(restart_text,
                (screen_width - text_width) / 2,
                y_pos,
                results_font_size * 0.6f,
                ColorAlpha(LIGHTGRAY, alpha * 0.8f));
    }
}

void EndGame::try_spawn_winner_particles() {
    auto winner_it = std::find_if(m_results.begin(), m_results.end(), 
        [](const PlayerResult& r) { return r.winner; });
    
    if (winner_it == m_results.end()) return;
    
    auto particle_system = Query::try_find_first<ParticleSystem>();
    if (!particle_system) return;
    
    auto players = Query::find_where<PlayerInfo>([winner_index = winner_it->index](const PlayerInfo& p) {
        return p.index == winner_index;
    });
    
    if (players.empty()) return;
    
    auto winner_id = players[0].get().entity_id;
    auto winner_pos_opt = Query::try_get<Position>(winner_id);
    if (!winner_pos_opt) return;
    
    auto& winner_pos = winner_pos_opt->get();
    
    for (int i = 0; i < 3; i++) {
        Position start_pos = winner_pos;
        start_pos.x += get_random_value(-100, 100);
        start_pos.y += get_random_value(-100, 100);
        
        particle_system->get().spawn_teleport_effect(start_pos, winner_pos);
    }
    
    particle_system->get().spawn_collision_particles(
        winner_pos, 
        Position{winner_pos.x, winner_pos.y - 50},
        GOLD, 
        200
    );
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
