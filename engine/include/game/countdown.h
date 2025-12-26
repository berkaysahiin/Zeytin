#pragma once

#include "variant/variant_base.h"

class Countdown : public VariantBase {
    VARIANT(Countdown);

public:
    float duration = 60.0f; PROPERTY();
    bool countdown_active = true; PROPERTY();
    bool pause_when_zero = false; PROPERTY();
    
    int font_size = 48; PROPERTY();
    float offset_x = 50.0f; PROPERTY();
    float offset_y = 50.0f; PROPERTY();
    
    float warning_threshold = 30.0f; PROPERTY();
    float danger_threshold = 10.0f; PROPERTY();

    Color normal_color = {255, 255, 255, 255}; 
    Color warning_color = {255, 255, 0, 255}; 
    Color danger_color = {255, 0, 0, 255}; 
    
    virtual void on_init() override;
    virtual void on_update() override;
    virtual void on_play_start() override;
    virtual void on_play_update() override;

public:
    float get_remaining_time() const { return m_time_remaining; }
    bool is_finished() const { return m_time_remaining <= 0.0f; }
    void reset();
    void add_time(float seconds);

private:
    void draw_countdown();
    Color get_current_color() const;
    
    float m_time_remaining = 0.0f;
    bool m_has_finished = false;
};
