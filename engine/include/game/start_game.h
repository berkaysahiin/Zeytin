#pragma once

#include "variant/variant_base.h"
#include "core/raylib_wrapper.h"

class StartGame : public VariantBase {
    VARIANT(StartGame);

public:
    bool game_started;

    bool show_instructions = true; PROPERTY()
    bool show_controls = true; PROPERTY()
    float fade_in_duration = 1.0f; PROPERTY()
    float display_duration = 10.0f; PROPERTY() // How long to show the screen before auto-starting
    float background_opacity = 0.8f; PROPERTY()
    
    int title_font_size = 60; PROPERTY()
    int instruction_font_size = 28; PROPERTY()
    int controls_font_size = 24; PROPERTY()
    std::string game_title = "ZONE CONTROL"; PROPERTY()
    
    Color title_color = WHITE; PROPERTY()
    Color instruction_color = RAYWHITE; PROPERTY()
    Color highlight_color = YELLOW; PROPERTY()
    Color background_color = BLACK; PROPERTY()

    std::string instruction_text = "Stay in zones to gain points. Compete for the highest score!"; PROPERTY()
    
    virtual void on_init() override;
    virtual void on_play_update() override;

private:
    float m_timer = 0.0f;
    
    void draw_instructions();
    void draw_controls();
};
