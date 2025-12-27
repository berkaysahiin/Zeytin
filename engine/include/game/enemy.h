#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class Enemy : public VariantBase {
    VARIANT(Enemy);
    REQUIRES(Position, Scale, Collider);

public:
    float patrol_speed = 150.0f; PROPERTY();
    float patrol_distance = 200.0f; PROPERTY();
    
    float gravity = 1500.0f; PROPERTY();
    float max_fall_speed = 800.0f; PROPERTY();
    
    float shoot_interval = 2.0f; PROPERTY();  
    float shoot_range = 500.0f; PROPERTY();  
    
    float body_size = 50.0f; PROPERTY();
    float eye_size = 6.0f; PROPERTY();
    float eye_offset_x = 10.0f; PROPERTY();
    float eye_offset_y = -8.0f; PROPERTY();
    float eye_spacing = 8.0f; PROPERTY();
    
    float gun_length = 25.0f; PROPERTY();
    float gun_width = 8.0f; PROPERTY();
    float gun_offset_x = 15.0f; PROPERTY();
    float gun_offset_y = 5.0f; PROPERTY();

    virtual void on_play_start() override;
    virtual void on_update() override;
    virtual void on_play_update() override;

private:
    Color body_color = {150, 50, 50, 255};
    Color eye_color = {200, 0, 0, 255};
    Color gun_color = {80, 80, 80, 255};

    void patrol();
    void apply_physics();
    void check_ground();
    void handle_shooting();
    void shoot();
    void draw_enemy();
    
    float m_start_x = 0.0f;
    int m_patrol_direction = 1;
    Vector2 m_velocity = {0.0f, 0.0f};
    bool m_is_grounded = false;
    
    float m_shoot_timer = 0.0f;
};
