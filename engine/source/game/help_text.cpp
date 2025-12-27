#include "game/help_text.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/position.h"

void HelpText::on_init() {
    m_timer = 0.0f;
    m_alpha = 0.0f;
    m_is_showing = false;
    m_fade_state = FadeState::Hidden;
    
    if (fade_in_duration > 0.0f || display_duration > 0.0f) {
        show();
    }
}

void HelpText::on_update() {
    update_fade();
    
    if (m_alpha > 0.0f) {
        draw_text_display();
    }
}

void HelpText::show() {
    m_is_showing = true;
    m_fade_state = FadeState::FadeIn;
    m_timer = 0.0f;
}

void HelpText::hide() {
    m_is_showing = false;
    m_fade_state = FadeState::FadeOut;
    m_timer = 0.0f;
}

void HelpText::update_fade() {
    if (!m_is_showing && m_fade_state == FadeState::Hidden) {
        return;
    }
    
    float delta = get_frame_time();
    m_timer += delta;
    
    switch (m_fade_state) {
        case FadeState::FadeIn:
            if (fade_in_duration > 0.0f) {
                m_alpha = m_timer / fade_in_duration;
                if (m_alpha >= 1.0f) {
                    m_alpha = 1.0f;
                    m_fade_state = FadeState::Display;
                    m_timer = 0.0f;
                }
            } else {
                m_alpha = 1.0f;
                m_fade_state = FadeState::Display;
                m_timer = 0.0f;
            }
            break;
            
        case FadeState::Display:
            m_alpha = 1.0f;
            if (display_duration > 0.0f && m_timer >= display_duration) {
                m_fade_state = FadeState::FadeOut;
                m_timer = 0.0f;
            }
            break;
            
        case FadeState::FadeOut:
            if (fade_out_duration > 0.0f) {
                m_alpha = 1.0f - (m_timer / fade_out_duration);
                if (m_alpha <= 0.0f) {
                    m_alpha = 0.0f;
                    m_fade_state = FadeState::Hidden;
                    m_timer = 0.0f;
                    
                    if (auto_destroy_after_fade) {
                        is_dead = true;
                    }
                }
            } else {
                m_alpha = 0.0f;
                m_fade_state = FadeState::Hidden;
                m_timer = 0.0f;
                
                if (auto_destroy_after_fade) {
                    is_dead = true;
                }
            }
            break;
            
        case FadeState::Hidden:
            m_alpha = 0.0f;
            break;
    }
}

void HelpText::draw_text_display() {
    auto& position = Query::get<Position>(this);
    
    int text_width = MeasureText(text.c_str(), font_size);
    
    float x = position.x;
    float y = position.y;
    
    if (center_on_screen || center_horizontally) {
        x = (VIRTUAL_WIDTH - text_width) / 2.0f;
    }
    
    if (center_on_screen || center_vertically) {
        y = (VIRTUAL_HEIGHT - font_size) / 2.0f;
    }
    
    if (show_background) {
        float bg_x = x - background_padding;
        float bg_y = y - background_padding;
        float bg_width = text_width + (background_padding * 2);
        float bg_height = font_size + (background_padding * 2);
        
        unsigned char bg_alpha = (unsigned char)(background_color.a * m_alpha);
        Color faded_bg = {
            background_color.r,
            background_color.g,
            background_color.b,
            bg_alpha
        };
        
        DrawRectangleRounded(
            {bg_x, bg_y, bg_width, bg_height},
            background_border_radius / bg_height,
            10,
            faded_bg
        );
    }
    
    unsigned char text_alpha = (unsigned char)(text_color.a * m_alpha);
    Color faded_text = {
        text_color.r,
        text_color.g,
        text_color.b,
        text_alpha
    };
    
    draw_text(text.c_str(), x, y, font_size, faded_text);
}
