#pragma once

#include "raylib.h"
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

    float squash_amount = 0.3f; PROPERTY();
    float stretch_amount = 0.3f; PROPERTY();
    float squash_speed = 8.0f; PROPERTY();
    
    float hat_offset_y = -35.0f; PROPERTY();
    float hat_base_width = 40.0f; PROPERTY();
    float hat_height = 30.0f; PROPERTY();
    Color hat_color = {200, 0, 0, 255}; PROPERTY();
    Color hat_trim_color = {255, 255, 255, 255}; PROPERTY();
    Color hat_pom_color = {255, 255, 255, 255}; PROPERTY();
    float hat_trim_height = 5.0f; PROPERTY();
    float hat_pom_radius = 6.0f; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_play_update() override;
    virtual void on_update() override;
    
    bool is_grounded() const { return m_is_grounded; }
    bool has_diffuser() const { return m_has_diffuser; }
    void set_has_diffuser(bool has) { m_has_diffuser = has; }

    Vector2 get_velocity() { return m_velocity; }

private:
    Color body_color; 
    Color eye_color; 

    void handle_input();
    void apply_physics();
    void check_horizontal_collision();
    void draw_character();
    void draw_diffuser_icon();
    void draw_christmas_hat();
    void check_ground();
    void check_enemy_collision();
    void check_bullet_collision();
    void update_squash_stretch();  
    
    Vector2 m_velocity = {0.0f, 0.0f};
    bool m_is_grounded = false;
    bool m_jump_pressed_last_frame = false;
    int m_facing_direction = 1;
    int m_jumps_remaining = 0;
    bool m_has_diffuser = false;
    
    bool m_was_grounded_last_frame = false;
    bool m_just_jumped = false;
    Vector2 m_target_scale = {1.0f, 1.0f};  
};
