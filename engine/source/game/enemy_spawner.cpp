#include "game/enemy_spawner.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/enemy.h"
#include "game/player.h"
#include "game/position.h"
#include "game/scale.h"
#include "game/collider.h"
#include "game/game_manager.h"
#include <cmath>

float min(float a, float b) { return a < b ? a : b; }

void EnemySpawner::on_init() {
    m_initial_spawned = false;
    m_spawn_timer = 0.0f;
    m_total_spawned = 0;
}

void EnemySpawner::on_update() {
    if (show_debug_spawner) {
        draw_debug();
    }
}

void EnemySpawner::on_play_update() {
    auto game_mgr_opt = Query::try_find_first<GameManager>();
    if (!game_mgr_opt || !game_mgr_opt->get().should_game_run()) {
        return;
    }
    
    if (!m_initial_spawned) {
        m_initial_delay_timer += get_frame_time();
        
        if (m_initial_delay_timer >= initial_spawn_delay) {
            spawn_initial_bulk();
            m_initial_spawned = true;
        }
        return;
    }
    
    if (!keep_spawning) {
        return;
    }
    
    m_spawn_timer += get_frame_time();
    
    if (m_spawn_timer >= spawn_every_seconds) {
        int alive = count_alive_enemies();
        if (alive < max_alive_at_once) {
            spawn_one_enemy();
        }
        m_spawn_timer = 0.0f;
    }
}

void EnemySpawner::spawn_initial_bulk() {
    for (int i = 0; i < initial_enemy_count; i++) {
        Vector2 spawn_pos = find_good_spawn_position(100); // More attempts for initial spawn
        
        if (!is_position_too_close_to_player(spawn_pos)) {
            spawn_one_enemy_at(spawn_pos);
        }
    }
}

void EnemySpawner::spawn_one_enemy() {
    Vector2 spawn_pos = find_good_spawn_position(50);
    
    if (!is_position_too_close_to_player(spawn_pos)) {
        spawn_one_enemy_at(spawn_pos);
    }
}

void EnemySpawner::spawn_one_enemy_at(Vector2 spawn_pos) {
    ::entity_id enemy_id = Query::create_entity();
    
    auto pos_opt = Query::add<Position>(enemy_id);
    if (pos_opt) {
        auto& pos = pos_opt->get();
        pos.x = spawn_pos.x;
        pos.y = spawn_pos.y;
    }
    
    auto scale_opt = Query::add<Scale>(enemy_id);
    if (scale_opt) {
        auto& scale = scale_opt->get();
        scale.x = 1.0f;
        scale.y = 1.0f;
    }
    
    auto collider_opt = Query::add<Collider>(enemy_id);
    if (collider_opt) {
        auto& collider = collider_opt->get();
        collider.width = 50.0f;
        collider.height = 50.0f;
    }
    
    auto enemy_opt = Query::add<Enemy>(enemy_id);
    if (enemy_opt) {
        auto& enemy = enemy_opt->get();
        enemy.use_global_config = true;
    }
    
    m_total_spawned++;
}

Vector2 EnemySpawner::find_good_spawn_position(int max_attempts) {
    float half_width = spawn_area_width / 2.0f;
    float half_height = spawn_area_height / 2.0f;
    
    Vector2 best_pos = {0, 0};
    float best_player_distance = 0.0f;
    bool found_any_valid = false;
    
    for (int attempt = 0; attempt < max_attempts; attempt++) {
        float x = spawn_area_center_x + get_random_value(static_cast<int>(-half_width), static_cast<int>(half_width));
        float y = spawn_area_center_y + get_random_value(static_cast<int>(-half_height), static_cast<int>(half_height));
        
        Vector2 pos = {x, y};
        
        bool too_close_to_enemies = is_position_too_close_to_enemies(pos);
        bool too_close_to_player = is_position_too_close_to_player(pos);
        
        if (!too_close_to_enemies && !too_close_to_player) {
            return pos;
        }
        
        if (!too_close_to_enemies) {
            auto player_opt = Query::try_find_first<Player>();
            if (player_opt) {
                auto& player_pos = Query::get<Position>(&player_opt->get());
                float dx = pos.x - player_pos.x;
                float dy = pos.y - player_pos.y;
                float dist = sqrt(dx * dx + dy * dy);
                
                if (!found_any_valid || dist > best_player_distance) {
                    best_pos = pos;
                    best_player_distance = dist;
                    found_any_valid = true;
                }
            }
        }
    }
    
    if (found_any_valid) {
        return best_pos;
    }
    
    auto player_opt = Query::try_find_first<Player>();
    if (player_opt) {
        auto& player_pos = Query::get<Position>(&player_opt->get());
        
        // Spawn on opposite side of the spawn area from player
        float angle_to_player = atan2(player_pos.y - spawn_area_center_y, 
                                       player_pos.x - spawn_area_center_x);
        float opposite_angle = angle_to_player + 3.14159f; // PI radians = opposite direction
        
        float distance = min(half_width, half_height) * 0.8f;
        float x = spawn_area_center_x + cos(opposite_angle) * distance;
        float y = spawn_area_center_y + sin(opposite_angle) * distance;
        
        return {x, y};
    }
    
    return {spawn_area_center_x, spawn_area_center_y};
}

bool EnemySpawner::is_position_too_close_to_enemies(Vector2 pos) {
    auto enemy_entities = Query::find_all_with<Enemy, Position>();
    
    for (::entity_id enemy_id : enemy_entities) {
        auto& enemy = Query::get<Enemy>(enemy_id);
        if (enemy.is_dead()) continue;
        
        auto& enemy_pos = Query::get<Position>(enemy_id);
        
        float dx = pos.x - enemy_pos.x;
        float dy = pos.y - enemy_pos.y;
        float distance = sqrt(dx * dx + dy * dy);
        
        if (distance < min_distance_between_enemies) {
            return true; 
        }
    }
    
    return false; }

bool EnemySpawner::is_position_too_close_to_player(Vector2 pos) {
    auto player_opt = Query::try_find_first<Player>();
    if (!player_opt) {
        return false;     }
    
    auto& player_pos = Query::get<Position>(&player_opt->get());
    
    float dx = pos.x - player_pos.x;
    float dy = pos.y - player_pos.y;
    float distance = sqrt(dx * dx + dy * dy);
    
    if (distance < player_safe_zone_radius) {
        return true; // Too close to player!
    }

    return false; // Position is safe
}

int EnemySpawner::count_alive_enemies() const {
    int count = 0;
    auto enemy_entities = Query::find_all_with<Enemy>();
    
    for (::entity_id enemy_id : enemy_entities) {
        auto& enemy = Query::get<Enemy>(enemy_id);
        if (!enemy.is_dead()) {
            count++;
        }
    }
    
    return count;
}

void EnemySpawner::draw_debug() {
    float half_width = spawn_area_width / 2.0f;
    float half_height = spawn_area_height / 2.0f;
    
    draw_rectangle_lines(
        spawn_area_center_x - half_width,
        spawn_area_center_y - half_height,
        spawn_area_width,
        spawn_area_height,
        {0, 255, 0, 150}
    );
    
    draw_circle(spawn_area_center_x, spawn_area_center_y, 10.0f, {255, 0, 0, 200});
    
    // Draw player safe zone
    auto player_opt = Query::try_find_first<Player>();
    if (player_opt) {
        auto& player_pos = Query::get<Position>(&player_opt->get());
        draw_circle_lines(player_pos.x, player_pos.y, player_safe_zone_radius, {255, 255, 0, 200});
    }
    
    const char* info = TextFormat("Alive: %d/%d | Total: %d", 
                                  count_alive_enemies(), 
                                  max_alive_at_once,
                                  m_total_spawned);
    draw_text(info, 10, 10, 20, WHITE);
}
