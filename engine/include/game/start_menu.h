// engine/include/game/start_game.h
#pragma once
#include "variant/variant_base.h"
#include "core/raylib_wrapper.h"

class StartMenu : public VariantBase {
    VARIANT(StartMenu);

public:
    bool show_instructions = true;
    float fade_in_duration = 1.0f; 
    
    int title_font_size = 48; PROPERTY();
    int instruction_font_size = 24; 
    
    std::string game_title = "BOMB DEFUSAL"; 
    
    Color title_color = WHITE; 
    Color instruction_color = RAYWHITE; 
    Color highlight_color = {255, 100, 0, 255}; 
    Color background_color = BLACK; 
    float background_opacity = 0.85f; 
    
    virtual void on_init() override;
    virtual void on_update() override;
    virtual void on_play_update() override;
    
    bool has_game_started() const { return m_game_started; }

private:
    float m_timer = 0.0f;
    bool m_game_started = false;
    
    void draw_instructions();
};
