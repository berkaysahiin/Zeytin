#pragma once

#include "variant/variant_base.h"

class PlayerInfo : public VariantBase {
    VARIANT(PlayerInfo);

public:
    int index = 0; PROPERTY();
    std::string name; PROPERTY();
    Color color = BLUE; PROPERTY();
    bool in_zone = false; PROPERTY(); 
    float time_spent_zone = 0; PROPERTY();

    // ui
    float m_panel_width = 200.0f; PROPERTY();
    float m_panel_height = 160.0f; PROPERTY();
    float m_padding = 0; PROPERTY();
    float m_panel_x = 0; PROPERTY();
    float m_panel_y = 30.0f; PROPERTY();

    virtual void on_post_init() override;
    virtual void on_play_update() override;

private:
    void draw_ui();
};


