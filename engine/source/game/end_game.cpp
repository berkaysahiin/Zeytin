#include "game/end_game.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "core/zeytin.h"
#include "game/particle_system.h"
#include "game/zone.h"
#include "game/character_controller.h"
#include "remote_logger/remote_logger.h"

void EndGame::on_init() {
    m_game_over = false;
    m_post_game_timer = 0.0f;
    is_game_active = true;
}

void EndGame::on_play_update() {
    if (!is_game_active) return;
    
    if (!m_game_over) {
        check_victory_conditions();
    } else {
        m_post_game_timer += get_frame_time();
        
        if (show_winner_screen) {
            draw_game_over_screen();
            draw_results();
        }
        
        if (m_post_game_timer >= post_game_delay) {
        }
    }
}

void EndGame::check_victory_conditions() {
    
}

void EndGame::calculate_results() {
    m_results.clear();
    
    auto player_infos = Query::find_all<PlayerInfo>();
    if (player_infos.empty()) return;
    
    float max_score = 0.0f;
    for (auto& player_ref : player_infos) {
        auto& player = player_ref.get();
        max_score = std::max(max_score, player.time_spent_zone);
    }
    
    for (auto& player_ref : player_infos) {
        auto& player = player_ref.get();
        
        PlayerResult result;
        result.index = player.index;
        result.name = player.name.empty() ? "Player " + std::to_string(player.index + 1) : player.name;
        result.score = player.time_spent_zone;
        result.color = player.color;
        result.winner = (player.time_spent_zone == max_score);
        
        m_results.push_back(result);
    }
    
    std::sort(m_results.begin(), m_results.end(), 
        [](const PlayerResult& a, const PlayerResult& b) {
            return a.score > b.score;
        });
}

void EndGame::draw_game_over_screen() {
    float screen_width = VIRTUAL_WIDTH;
    float screen_height = VIRTUAL_HEIGHT;
    
    draw_rectangle(0, 0, screen_width, screen_height, 
                 ColorAlpha(BLACK, ui_transparency));
    
    const char* game_over_text = "GAME OVER";
    int text_width = MeasureText(game_over_text, game_over_font_size);
    
    draw_text(game_over_text,
              (screen_width - text_width) / 2 + 2,
              screen_height * 0.25f + 2,
              game_over_font_size,
              BLACK);
    
    draw_text(game_over_text,
              (screen_width - text_width) / 2,
              screen_height * 0.25f,
              game_over_font_size,
              WHITE);
}

void EndGame::draw_results() {
    float screen_width = VIRTUAL_WIDTH;
    float screen_height = VIRTUAL_HEIGHT;
    
    float start_y = screen_height * 0.4f;
    float line_height = results_font_size * 1.5f;
    
    for (size_t i = 0; i < m_results.size(); i++) {
        const PlayerResult& result = m_results[i];
        float y_pos = start_y + i * line_height;
        
        char result_text[100];
        sprintf(result_text, "%s: %.1f", 
                result.name.c_str(), 
                result.score);
        
        Color text_color = result.color;
        if (result.winner) {
            // Make winner stand out with pulsing effect
            float pulse = 0.8f + 0.2f * sinf(get_time() * 3.0f);
            text_color = ColorAlpha(result.color, pulse);
            
            // Draw winner indicator
            const char* winner_text = "WINNER!";
            int winner_width = MeasureText(winner_text, results_font_size);
            draw_text(winner_text,
                      screen_width * 0.7f,
                      y_pos,
                      results_font_size,
                      text_color);
        }
        
        draw_text(result_text,
                  screen_width * 0.3f,
                  y_pos,
                  results_font_size,
                  text_color);
    }
    
    if (m_post_game_timer <= post_game_delay) {
        char restart_text[100];
        sprintf(restart_text, "Returning to menu in %.1f seconds...", 
                post_game_delay - m_post_game_timer);
        
        int text_width = MeasureText(restart_text, results_font_size / 2);
        float y_pos = screen_height * 0.7f;
        
        draw_text(restart_text,
                  (screen_width - text_width) / 2,
                  y_pos,
                  results_font_size / 2,
                  GRAY);
    }
}

void EndGame::end_game() {
    if (m_game_over) return;
    
    m_game_over = true;
    calculate_results();
    
    auto controllers = Query::find_all<CharacterController>();
    for (auto& controller_ref : controllers) {
        controller_ref.get().is_dead = true;
    }
    
    if (!m_results.empty() && m_results[0].winner) {
        auto particle_system = Query::try_find_first<ParticleSystem>();
        if (particle_system) {
            auto players = Query::find_where<PlayerInfo>([&](const PlayerInfo& p) {
                return p.index == m_results[0].index;
            });
            
            if (!players.empty()) {
                auto& winner = players[0].get();
                auto winner_pos = Query::get<Position>(winner.entity_id);
                
                for (int i = 0; i < 5; i++) {
                    Position start_pos = winner_pos;
                    start_pos.x += get_random_value(-50, 50);
                    start_pos.y += get_random_value(-50, 50);
                    
                    Position end_pos = winner_pos;
                    
                    particle_system->get().spawn_teleport_effect(start_pos, end_pos);
                }
            }
        }
    }
    
    log_info() << "Game Over! Results calculated." << std::endl;
}

void EndGame::restart_game() {
    m_game_over = false;
    m_post_game_timer = 0.0f;
    
    auto player_infos = Query::find_all<PlayerInfo>();
    for (auto& player_ref : player_infos) {
        auto& player = player_ref.get();
        player.time_spent_zone = 0.0f;
    }
    
    log_info() << "Game Restarted!" << std::endl;
}
