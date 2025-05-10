#pragma once

#include "variant/variant_base.h"

class PlayerInfo : public VariantBase {
    VARIANT(PlayerInfo);

public:
    int index = 0; PROPERTY();
    std::string name; PROPERTY();
    Color color = BLUE; PROPERTY();
    float time_spent_zone = 0; PROPERTY();

    bool in_zone = false; PROPERTY(); 
    bool shield = false; PROPERTY();
    
    float m_panel_width = 280.0f; PROPERTY();
    float m_panel_height = 160.0f; PROPERTY();
    float m_padding = 15.0f; PROPERTY();
    float m_panel_x = 20.0f; PROPERTY();
    float m_panel_y = 30.0f; PROPERTY();

    float max_time_outside = 12; PROPERTY();
    float warning_threshold = 3.0f; PROPERTY();
    float danger_threshold = 6.0f; PROPERTY();
    float critical_threshold = 9.0f; PROPERTY();

    float since_last_zone = 0;

    virtual void on_post_init() override;
    virtual void on_play_update() override;

private:
    void draw_ui();
    void draw_zone_warning();
};
