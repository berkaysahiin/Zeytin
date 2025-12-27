#include "game/enemy.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/position.h"
#include "game/scale.h"
#include "game/collider.h"
#include "game/obstacle.h"
#include "game/player.h"
#include "game/bullet.h"

void Enemy::on_play_start() {
    auto& position = Query::get<Position>(this);
    m_start_x = position.x;
    m_patrol_direction = 1;
    m_velocity = {0.0f, 0.0f};
    m_is_grounded = false;
    m_shoot_timer = 0.0f;
}

void Enemy::on_update() {
    draw_enemy();
}

void Enemy::on_play_update() {
    patrol();
    apply_physics();
    check_ground();
    handle_shooting();
}

void Enemy::patrol() {
    auto& position = Query::get<Position>(this);
    
    if (!m_is_grounded) {
        m_velocity.x = 0.0f;
        return;
    }
    
    float left_bound = m_start_x - patrol_distance / 2;
    float right_bound = m_start_x + patrol_distance / 2;
    
    m_velocity.x = m_patrol_direction * patrol_speed;
    
    if (m_patrol_direction == -1 && position.x <= left_bound) {
        m_patrol_direction = 1;
    } else if (m_patrol_direction == 1 && position.x >= right_bound) {
        m_patrol_direction = -1;
    }
}

void Enemy::apply_physics() {
    auto& position = Query::get<Position>(this);
    float delta = get_frame_time();
    
    if (!m_is_grounded) {
        m_velocity.y += gravity * delta;
        if (m_velocity.y > max_fall_speed) {
            m_velocity.y = max_fall_speed;
        }
    }
    
    position.x += m_velocity.x * delta;
    position.y += m_velocity.y * delta;
}

void Enemy::check_ground() {
    auto& position = Query::get<Position>(this);
    auto& enemy_collider = Query::get<Collider>(this);
    
    float ground_level = VIRTUAL_HEIGHT - body_size / 2;
    bool landed = false;
    
    Rectangle enemy_bounds = enemy_collider.get_bounds();
    float enemy_bottom = position.y + enemy_collider.height / 2;
    
    auto obstacle_entities = Query::find_all_with<Collider, Obstacle>();
    
    for (::entity_id obstacle_id : obstacle_entities) {
        auto& obstacle_collider = Query::get<Collider>(obstacle_id);
        Rectangle obstacle_bounds = obstacle_collider.get_bounds();
        
        if (m_velocity.y >= 0 &&
            enemy_bounds.x + enemy_bounds.width > obstacle_bounds.x &&
            enemy_bounds.x < obstacle_bounds.x + obstacle_bounds.width &&
            enemy_bottom >= obstacle_bounds.y &&
            enemy_bottom <= obstacle_bounds.y + 10) {
            
            position.y = obstacle_bounds.y - enemy_collider.height / 2;
            m_velocity.y = 0.0f;
            landed = true;
            break;
        }
    }
    
    if (!landed && position.y >= ground_level) {
        position.y = ground_level;
        m_velocity.y = 0.0f;
        landed = true;
    }
    
    m_is_grounded = landed;
}

void Enemy::handle_shooting() {
    float delta = get_frame_time();
    m_shoot_timer += delta;
    
    if (m_shoot_timer >= shoot_interval) {
        // Find player using try_find_first
        auto player_opt = Query::try_find_first<Player>();
        if (player_opt) {
            auto& player = player_opt->get();
            auto& enemy_pos = Query::get<Position>(this);
            auto& player_pos = Query::get<Position>(&player);
            
            // Check if player is in range
            float distance = abs(player_pos.x - enemy_pos.x);
            if (distance <= shoot_range) {
                // Aim at player
                if (player_pos.x < enemy_pos.x) {
                    m_patrol_direction = -1;
                } else {
                    m_patrol_direction = 1;
                }
                
                shoot();
                m_shoot_timer = 0.0f;
            }
        }
    }
}

void Enemy::shoot() {
    auto& enemy_pos = Query::get<Position>(this);
    auto& enemy_scale = Query::get<Scale>(this);
    
    // Calculate bullet spawn position (at gun tip)
    float scaled_gun_offset_x = gun_offset_x * enemy_scale.x * m_patrol_direction;
    float scaled_gun_length = gun_length * enemy_scale.x;
    float scaled_gun_offset_y = gun_offset_y * enemy_scale.y;
    
    float bullet_x = enemy_pos.x + scaled_gun_offset_x + (m_patrol_direction * scaled_gun_length);
    float bullet_y = enemy_pos.y + scaled_gun_offset_y;
    
    // Create bullet entity
    ::entity_id bullet_id = Query::create_entity();
    
    // Add Position
    auto pos_opt = Query::add<Position>(bullet_id);
    if (pos_opt) {
        auto& bullet_pos = pos_opt->get();
        bullet_pos.x = bullet_x;
        bullet_pos.y = bullet_y;
    }
    
    // Add Scale
    auto scale_opt = Query::add<Scale>(bullet_id);
    if (scale_opt) {
        auto& bullet_scale = scale_opt->get();
        bullet_scale.x = 1.0f;
        bullet_scale.y = 1.0f;
    }
    
    // Add Collider (will be sized by Bullet component)
    Query::add<Collider>(bullet_id);
    
    // Add Bullet component with direction
    auto bullet_opt = Query::add<Bullet>(bullet_id);
    if (bullet_opt) {
        auto& bullet = bullet_opt->get();
        bullet.m_direction = m_patrol_direction;
    }
}

void Enemy::draw_enemy() {
    auto [position, scale] = Query::read<Position, Scale>(this);
    
    float scaled_size = body_size * scale.x;
    
    Rectangle body_rect = {
        position.x - scaled_size / 2,
        position.y - scaled_size / 2,
        scaled_size,
        scaled_size
    };
    draw_rectangle_rec(body_rect, body_color);
    
    float scaled_eye_size = eye_size * scale.x;
    float scaled_eye_offset_x = eye_offset_x * scale.x * m_patrol_direction;
    float scaled_eye_offset_y = eye_offset_y * scale.y;
    float scaled_eye_spacing = eye_spacing * scale.x;
    
    draw_circle(
        position.x + scaled_eye_offset_x - scaled_eye_spacing,
        position.y + scaled_eye_offset_y,
        scaled_eye_size,
        eye_color
    );
    
    draw_circle(
        position.x + scaled_eye_offset_x + scaled_eye_spacing,
        position.y + scaled_eye_offset_y,
        scaled_eye_size,
        eye_color
    );
    
    float scaled_gun_length = gun_length * scale.x;
    float scaled_gun_width = gun_width * scale.y;
    float scaled_gun_offset_x = gun_offset_x * scale.x * m_patrol_direction;
    float scaled_gun_offset_y = gun_offset_y * scale.y;
    
    Rectangle gun_rect;
    if (m_patrol_direction == 1) {
        gun_rect = {
            position.x + scaled_gun_offset_x,
            position.y + scaled_gun_offset_y - scaled_gun_width / 2,
            scaled_gun_length,
            scaled_gun_width
        };
    } else {
        gun_rect = {
            position.x + scaled_gun_offset_x - scaled_gun_length,
            position.y + scaled_gun_offset_y - scaled_gun_width / 2,
            scaled_gun_length,
            scaled_gun_width
        };
    }
    
    draw_rectangle_rec(gun_rect, gun_color);
}
