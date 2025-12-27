#pragma once

#include "variant/variant_base.h"

class Bomb : public VariantBase {
    VARIANT(Bomb);
    REQUIRES(Position, Scale, Countdown);

public:
    float defuse_time = 5.0f; PROPERTY();
    float defuse_radius = 150.0f; PROPERTY();
    
    float bomb_size = 40.0f; PROPERTY();
    
    // Defuse bar visual properties
    float bar_width = 60.0f; PROPERTY();
    float bar_height = 8.0f; PROPERTY();
    float bar_offset_y = -30.0f; PROPERTY();
    float bar_border_thickness = 2.0f; PROPERTY();
    float bar_outline_thickness = 1.0f; PROPERTY();
    int bar_progress_font_size = 14; PROPERTY();
    float bar_text_offset_y = 5.0f; PROPERTY();
    float bar_text_shadow_offset = 1.0f; PROPERTY();
    
    // Scary effect parameters
    float pulse_speed = 3.0f; PROPERTY();
    float pulse_intensity = 0.2f; PROPERTY();
    float shake_intensity = 2.0f; PROPERTY();
    float glow_size = 60.0f; PROPERTY();
    bool show_danger_rings = true; PROPERTY();
    float ring_speed = 100.0f; PROPERTY();
    int label_size = 20; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_update() override;
    virtual void on_play_update() override;
    
    bool is_defused() const { return m_is_defused; }
    bool is_being_defused() const { return m_player_defusing; }
    float get_defuse_progress() const { return m_defuse_progress; }
	void set_defuse_progress(float progress) { m_defuse_progress = progress; }

private:
    Color bomb_color = {50, 50, 50, 255};
    Color circle_color = {255, 165, 0, 80};
    Color bar_bg_color = {60, 60, 60, 200};
    
    bool m_is_defused = false;
    float m_defuse_progress = 0.0f;
    bool m_player_in_range = false;
    bool m_player_missing_diffuser = false;
	bool m_player_defusing = false;
    float m_timer = 0.0f;
    float m_ring_timer = 0.0f;
    
    void check_player_in_range();
    void update_defuse();
    void draw_bomb();
    void draw_defuse_circle();
    void draw_defuse_bar();
    void draw_danger_effects();
    void draw_red_glow();
    void draw_danger_rings();
    Color get_bar_color() const;
    float get_danger_level() const;
};
