#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class Enemy : public VariantBase {
    VARIANT(Enemy);
    REQUIRES(Position, Scale, Collider);

public:
    bool use_global_config = true; PROPERTY(); 

	// if use global config is set to false, use below

    float patrol_speed = -1; PROPERTY();
    float patrol_distance = -1; PROPERTY();
    
    float gravity = -1; PROPERTY();
    float max_fall_speed = -1; PROPERTY();
    
    float shoot_interval = -1; PROPERTY();  
    float shoot_range = -1; PROPERTY();  
    
    float body_size = -1; PROPERTY();
    float eye_size = -1; PROPERTY();
    float eye_offset_x = -1; PROPERTY();
    float eye_offset_y = -1; PROPERTY();
    float eye_spacing = -1; PROPERTY();
    
    float gun_length = -1; PROPERTY();
    float gun_width = -1; PROPERTY();
    float gun_offset_x = -1; PROPERTY();
    float gun_offset_y = -1; PROPERTY();

    float death_fade_duration = -1; PROPERTY();

    virtual void on_play_start() override;
    virtual void on_update() override;
    virtual void on_play_update() override;

	Vector2 get_velocity() const { return m_velocity; }
	void set_velocity(Vector2 v) { m_velocity = v; }

	bool get_is_grounded() const { return m_is_grounded; }
	void set_is_grounded(bool grounded) { m_is_grounded = grounded; }

	int get_patrol_direction() const { return m_patrol_direction; }
	void set_patrol_direction(int dir) { m_patrol_direction = dir; }

	float get_shoot_timer() const { return m_shoot_timer; }
	void set_shoot_timer(float timer) { m_shoot_timer = timer; }

    void kill();
    bool is_dead() const { return m_is_dead; }
	void set_is_dead(bool dead) { m_is_dead = dead; }

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
    void update_death_animation();
    
    float m_start_x = 0.0f;
    int m_patrol_direction = 1;
    Vector2 m_velocity = {0.0f, 0.0f};
    bool m_is_grounded = false;
    
    float m_shoot_timer = 0.0f;
    
    bool m_is_dead = false;
    float m_death_timer = 0.0f;
};
