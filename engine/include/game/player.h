#pragma once

#include "variant/variant_base.h"

class Player : public VariantBase {
    VARIANT(Player);
    REQUIRES(Position, Scale, Collider);

public:
    float move_speed = 300.0f; PROPERTY();
    float jump_force = 500.0f; PROPERTY();
    float gravity = 1500.0f; PROPERTY();
    float max_fall_speed = 800.0f; PROPERTY();
    
    bool enable_double_jump = true; PROPERTY();
    int max_jumps = 2; PROPERTY();
    
    float body_size = 50.0f; PROPERTY();

    float eye_size = 8.0f; PROPERTY();
    float eye_offset_x = 12.0f; PROPERTY();
    float eye_offset_y = -8.0f; PROPERTY();
    float eye_spacing = 10.0f; PROPERTY();

    virtual void on_init() override;
    virtual void on_update() override;
    virtual void on_play_update() override;

private:
    Color body_color; 
    Color eye_color; 

    void handle_input();
    void apply_physics();
    void draw_character();
    void check_ground();
    void check_enemy_collision();
    void check_bullet_collision();
    void check_game_over();
    
    Vector2 m_velocity = {0.0f, 0.0f};
    bool m_is_grounded = false;
    bool m_jump_pressed_last_frame = false;
    int m_facing_direction = 1; // 1 = right, -1 = left
    int m_jumps_remaining = 0;
};
