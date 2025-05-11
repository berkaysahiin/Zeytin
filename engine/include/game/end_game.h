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
    
    bool is_game_active = true; PROPERTY();
    
    bool show_countdown = true; PROPERTY();
    bool show_winner_screen = true; PROPERTY();
    int countdown_font_size = 60; PROPERTY();
    int game_over_font_size = 80; PROPERTY();
    int results_font_size = 30; PROPERTY();
    float ui_transparency = 0.85f; PROPERTY();
    float post_game_delay = 5.0f; PROPERTY(); 
    
    void end_game(); 
    void restart_game(); 
    
private:
    std::vector<PlayerResult> m_results;
    float m_post_game_timer = 0.0f;
    bool m_game_over = false;
    
    void check_victory_conditions();
    void calculate_results();
    void draw_game_over_screen();
    void draw_results();
};
