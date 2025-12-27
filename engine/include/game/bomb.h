#pragma once

#include "variant/variant_base.h"

class Bomb : public VariantBase {
    VARIANT(Bomb);
    REQUIRES(Position, Scale);

public:
    float defuse_time = 5.0f; PROPERTY();
    float defuse_radius = 150.0f; PROPERTY();
    
    float bomb_size = 40.0f; PROPERTY();
    float bar_width = 60.0f; PROPERTY();
    float bar_height = 8.0f; PROPERTY();
    float bar_offset_y = -30.0f; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_update() override;
    virtual void on_play_update() override;
    
    bool is_defused() const { return m_is_defused; }
    float get_defuse_progress() const { return m_defuse_progress; }

private:
    Color bomb_color = {50, 50, 50, 255};
    Color circle_color = {255, 165, 0, 80};
    Color bar_bg_color = {60, 60, 60, 200};
    
    bool m_is_defused = false;
    float m_defuse_progress = 0.0f;
    bool m_player_in_range = false;
    bool m_player_missing_diffuser = false;
    
    void check_player_in_range();
    void update_defuse();
    void draw_bomb();
    void draw_defuse_circle();
    void draw_defuse_bar();
    Color get_bar_color() const;
};
