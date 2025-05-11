#pragma once

#include "variant/variant_base.h"
#include "game/player_info.h"
#include <vector>
#include <string>

struct PlayerResult {
    int index;
    std::string name;
    float score;
    Color color;
    bool winner;
};

class EndGame : public VariantBase {
    VARIANT(EndGame);

public:
    virtual void on_init() override;
    virtual void on_play_update() override;
    
    void mark_player_lost(int player_index, const std::string& reason = "");
    void end_game(const std::string& reason = "");
    void restart_game();
    
    bool show_countdown = true; PROPERTY();
    bool show_winner_screen = true; PROPERTY();
    int countdown_font_size = 60; PROPERTY();
    int game_over_font_size = 80; PROPERTY();
    int results_font_size = 30; PROPERTY();
    float ui_transparency = 0.85f; PROPERTY();
    float post_game_delay = 5.0f; PROPERTY(); 
    
private:
    std::vector<PlayerResult> m_results;
    float m_post_game_timer = 0.0f;
    float m_fade_timer = 0.0f;
    bool m_game_over = false;
    std::string m_lose_reason;
    
    void draw_game_over_screen();
    void draw_results(float alpha);
    void try_spawn_winner_particles();
    int get_keycode_pressed();
};
