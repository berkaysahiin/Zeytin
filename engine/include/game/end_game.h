#pragma once

#include "variant/variant_base.h"
#include <string>

class EndGame : public VariantBase {
    VARIANT(EndGame);

public:
    virtual void on_init() override;
    virtual void on_play_update() override;
    
    void trigger_game_over(const std::string& reason = "");
    void trigger_win();
    void restart_game();
    bool is_game_over() const { return m_game_over; }
    
    int game_over_font_size = 80; PROPERTY();
    int win_font_size = 80; PROPERTY();
    int reason_font_size = 30; PROPERTY();
    float ui_transparency = 0.85f; PROPERTY();
    
    std::string next_level = "FILL THIS FOR EVERY LEVEL"; PROPERTY();  
    
private:
    bool m_game_over = false;
    bool m_is_win = false;
    float m_post_game_timer = 0.0f;
    float m_fade_timer = 0.0f;
    std::string m_lose_reason;
    
    void draw_game_over_screen();
    void draw_win_screen();
    int get_keycode_pressed();
};
