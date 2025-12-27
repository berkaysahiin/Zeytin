#pragma once

#include "variant/variant_base.h"

class Diffuser : public VariantBase {
    VARIANT(Diffuser);
    REQUIRES(Position, Scale);

public:
    float pickup_radius = 80.0f; PROPERTY();
    float icon_size = 20.0f; PROPERTY();
    float icon_offset_y = -40.0f; PROPERTY();
    float bob_speed = 3.0f; PROPERTY();
    float bob_amount = 10.0f; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_update() override;
    virtual void on_play_update() override;
    
    bool is_picked_up() const { return m_picked_up; }

private:
    bool m_picked_up = false;
    float m_bob_timer = 0.0f;
    
    Color diffuser_color = {0, 255, 255, 255}; // Cyan
    Color pickup_circle_color = {0, 255, 255, 80};
    
    void check_player_pickup();
    void draw_diffuser();
    void draw_pickup_circle();
};
