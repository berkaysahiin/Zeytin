#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class Bullet : public VariantBase {
    VARIANT(Bullet);
    REQUIRES(Position, Scale, Collider);

public:
    virtual void on_init() override;
    virtual void on_update() override;
    virtual void on_play_update() override;

public:
    int m_direction = 1; 

private:
    void move();
    void draw_bullet();
    void check_lifetime();
    
	// adjusted via bullet_config
    float m_speed = 400.0f;
    float m_lifetime = 3.0f;
    float m_width = 8.0f;
    float m_height = 4.0f;

    Color m_bullet_color = {255, 200, 0, 255};
    
    float m_time_alive = 0.0f;
    bool m_config_loaded = false;
};
