#pragma once

#include "variant/variant_base.h"

class Retry : public VariantBase {
    VARIANT(Retry);

public:
    int button_font_size = 24; PROPERTY();
    float button_width = 200.0f; PROPERTY();
    float button_height = 60.0f; PROPERTY();
    float button_x = 10.0f; PROPERTY();  
    float button_y = 10.0f; PROPERTY(); 
    
    Color button_color = {60, 60, 60, 200}; 
    Color button_hover_color = {80, 80, 80, 220}; 
    Color text_color = WHITE; PROPERTY();
    
    virtual void on_update() override;
    virtual void on_play_update() override;

private:
    bool m_is_hovered = false;
    
    void draw_retry_button();
    bool is_button_clicked();
    void restart_game();
};
