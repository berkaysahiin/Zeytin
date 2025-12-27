#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class EnemySpawner : public VariantBase {
    VARIANT(EnemySpawner);
    REQUIRES(Position);

public:
    int initial_enemy_count = 5; PROPERTY();
    float initial_spawn_delay = 0.5f; PROPERTY(); 
    
    bool keep_spawning = true; PROPERTY();
    float spawn_every_seconds = 4.0f; PROPERTY();
    int max_alive_at_once = 8; PROPERTY();
    
    int enemy_type = 0; PROPERTY();
    
    float spawn_area_width = 1920.0f; PROPERTY();  // Full screen width by default
    float spawn_area_height = 1080.0f; PROPERTY(); // Full screen height by default
    float spawn_area_center_x = 960.0f; PROPERTY(); // Center X
    float spawn_area_center_y = 540.0f; PROPERTY(); // Center Y
    
    float min_distance_between_enemies = 100.0f; PROPERTY();
    
    float player_safe_zone_radius = 600.0f; PROPERTY();
    
    bool show_debug_spawner = false; PROPERTY();
    
    virtual void on_init() override;
    virtual void on_update() override;
    virtual void on_play_update() override;

private:
    bool m_initial_spawned = false;
    float m_spawn_timer = 0.0f;
    float m_initial_delay_timer = 0.0f;
    int m_total_spawned = 0;
    
    void spawn_initial_bulk();
    void spawn_one_enemy();
    void spawn_one_enemy_at(Vector2 spawn_pos);
    Vector2 find_good_spawn_position(int max_attempts = 50);
    bool is_position_too_close_to_enemies(Vector2 pos);
    bool is_position_too_close_to_player(Vector2 pos);
    int count_alive_enemies() const;
    void draw_debug();
};
