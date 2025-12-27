// help_text.h
#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class HelpText : public VariantBase {
    VARIANT(HelpText);
    REQUIRES(Position);

public:
    std::string text = "Press SPACE to continue"; PROPERTY();
    
    int font_size = 24; PROPERTY();
    Color text_color = {255, 255, 255, 255}; PROPERTY();
    Color background_color = {0, 0, 0, 180}; PROPERTY();
    
    bool show_background = true; PROPERTY();
    float background_padding = 20.0f; PROPERTY();
    float background_border_radius = 10.0f; PROPERTY();
    
    bool center_on_screen = false; PROPERTY();
    bool center_horizontally = false; PROPERTY();
    bool center_vertically = false; PROPERTY();
    
    float fade_in_duration = 0.5f; PROPERTY();
    float display_duration = 0.0f; PROPERTY();
    float fade_out_duration = 0.5f; PROPERTY();
    
    bool auto_destroy_after_fade = false; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_update() override;
    
    void show();
    void hide();
    bool is_visible() const { return m_alpha > 0.0f; }

private:
    void draw_text_display();
    void update_fade();
    
    float m_timer = 0.0f;
    float m_alpha = 0.0f;
    bool m_is_showing = false;
    
    enum class FadeState {
        FadeIn,
        Display,
        FadeOut,
        Hidden
    };
    
    FadeState m_fade_state = FadeState::Hidden;
};
