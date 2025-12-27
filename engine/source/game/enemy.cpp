#include "game/enemy.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/enemy_config.h"
#include "game/game_manager.h"
#include "game/position.h"
#include "game/scale.h"
#include "game/collider.h"
#include "game/obstacle.h"
#include "game/player.h"
#include "game/bullet.h"

void Enemy::on_play_start() {
	if (use_global_config) {
        auto config_opt = Query::try_find_first<EnemyConfig>();
        if (config_opt) {
            auto& config = config_opt->get();
            patrol_speed = config.patrol_speed;
            patrol_distance = config.patrol_distance;
            gravity = config.gravity;
            max_fall_speed = config.max_fall_speed;
            shoot_interval = config.shoot_interval;
            shoot_range = config.shoot_range;
            body_size = config.body_size;
            eye_size = config.eye_size;
            eye_offset_x = config.eye_offset_x;
            eye_offset_y = config.eye_offset_y;
            eye_spacing = config.eye_spacing;
            gun_length = config.gun_length;
            gun_width = config.gun_width;
            gun_offset_x = config.gun_offset_x;
            gun_offset_y = config.gun_offset_y;
            death_fade_duration = config.death_fade_duration;
        }
    }

    auto& position = Query::get<Position>(this);
    m_start_x = position.x;
    m_patrol_direction = 1;
    m_velocity = {0.0f, 0.0f};
    m_is_grounded = false;
    m_shoot_timer = 0.0f;
    m_is_dead = false;
    m_death_timer = 0.0f;
    m_death_position = {0.0f, 0.0f};
    m_skull_float_offset = 0.0f;
    m_death_rotation = 0.0f;
    m_death_fall_offset = 0.0f;
}

void Enemy::on_update() {
	if (use_global_config) {
        auto config_opt = Query::try_find_first<EnemyConfig>();
        if (config_opt) {
            auto& config = config_opt->get();
            patrol_speed = config.patrol_speed;
            patrol_distance = config.patrol_distance;
            gravity = config.gravity;
            max_fall_speed = config.max_fall_speed;
            shoot_interval = config.shoot_interval;
            shoot_range = config.shoot_range;
            body_size = config.body_size;
            eye_size = config.eye_size;
            eye_offset_x = config.eye_offset_x;
            eye_offset_y = config.eye_offset_y;
            eye_spacing = config.eye_spacing;
            gun_length = config.gun_length;
            gun_width = config.gun_width;
            gun_offset_x = config.gun_offset_x;
            gun_offset_y = config.gun_offset_y;
            death_fade_duration = config.death_fade_duration;
        }
    }

    draw_enemy();
    
    // Draw skull marker if dead
    if (m_is_dead && m_death_position.x != 0.0f) {
        draw_skull_marker();
    }
}

void Enemy::on_play_update() {
	auto game_mgr_opt = Query::try_find_first<GameManager>();
    if (!game_mgr_opt || !game_mgr_opt->get().should_game_run()) {
        return;
    }

    if (m_is_dead) {
        update_death_animation();
        return;
    }
    
    patrol();
    apply_physics();
    check_ground();
    handle_shooting();
}

void Enemy::kill() {
    if (m_is_dead) return;
    
    m_is_dead = true;
    m_death_timer = 0.0f;
    m_velocity = {0.0f, 0.0f};
    
    // Store death position
    auto& pos = Query::get<Position>(this);
    m_death_position = {pos.x, pos.y};
    m_skull_float_offset = 0.0f;
    m_death_rotation = 0.0f;
    m_death_fall_offset = 0.0f;
}

void Enemy::set_is_dead(bool dead) {
    // When revived (dead becomes false), clear the death position
    if (m_is_dead && !dead) {
        m_death_position = {0.0f, 0.0f};
    }
    m_is_dead = dead;
}

void Enemy::update_death_animation() {
    float delta = get_frame_time();
    m_death_timer += delta;
    
    // Lean over time (rotate 90 degrees over death_fade_duration)
    float rotation_progress = m_death_timer / death_fade_duration;
    if (rotation_progress > 1.0f) rotation_progress = 1.0f;
    
    // Ease out curve for smoother animation
    rotation_progress = 1.0f - (1.0f - rotation_progress) * (1.0f - rotation_progress);
    
    m_death_rotation = rotation_progress * 90.0f;  // 90 degrees = PI/2 radians worth
    
    // Fall down slightly as rotating
    m_death_fall_offset = rotation_progress * (body_size * 0.5f);
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
        auto player_opt = Query::try_find_first<Player>();
        if (player_opt) {
            auto& player = player_opt->get();
            auto& enemy_pos = Query::get<Position>(this);
            auto& player_pos = Query::get<Position>(&player);
            
            float distance = abs(player_pos.x - enemy_pos.x);
            if (distance <= shoot_range) {
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
    
    float scaled_gun_offset_x = gun_offset_x * enemy_scale.x * m_patrol_direction;
    float scaled_gun_length = gun_length * enemy_scale.x;
    float scaled_gun_offset_y = gun_offset_y * enemy_scale.y;
    
    float bullet_x = enemy_pos.x + scaled_gun_offset_x + (m_patrol_direction * scaled_gun_length);
    float bullet_y = enemy_pos.y + scaled_gun_offset_y;
    
    ::entity_id bullet_id = Query::create_entity();
    
    auto pos_opt = Query::add<Position>(bullet_id);
    if (pos_opt) {
        auto& bullet_pos = pos_opt->get();
        bullet_pos.x = bullet_x;
        bullet_pos.y = bullet_y;
    }
    
    auto scale_opt = Query::add<Scale>(bullet_id);
    if (scale_opt) {
        auto& bullet_scale = scale_opt->get();
        bullet_scale.x = 1.0f;
        bullet_scale.y = 1.0f;
    }
    
    Query::add<Collider>(bullet_id);
    
    auto bullet_opt = Query::add<Bullet>(bullet_id);
    if (bullet_opt) {
        auto& bullet = bullet_opt->get();
        bullet.m_direction = m_patrol_direction;
    }
}

void Enemy::draw_enemy() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float scaled_body_size = body_size * scale.x;
    float scaled_eye_size = eye_size * scale.x;
    float scaled_eye_offset_x = eye_offset_x * scale.x;
    float scaled_eye_offset_y = eye_offset_y * scale.y;
    float scaled_eye_spacing = eye_spacing * scale.x;
    float scaled_gun_length = gun_length * scale.x;
    float scaled_gun_width = gun_width * scale.y;
    float scaled_gun_offset_x = gun_offset_x * scale.x;
    float scaled_gun_offset_y = gun_offset_y * scale.y;
    
    // If dead and fully fallen, don't draw (skull marker is enough)
    if (m_is_dead && m_death_rotation >= 90.0f) {
        return;
    }
    
    // If dead but still falling, draw with rotation
    if (m_is_dead) {
        float draw_x = position.x;
        float draw_y = position.y + m_death_fall_offset;
        
        // Convert rotation to radians
        float rotation_rad = m_death_rotation * DEG2RAD;
        
        // Draw body as rotated rectangle
        Rectangle body_rect = {
            draw_x,
            draw_y,
            scaled_body_size,
            scaled_body_size
        };
        Vector2 body_origin = {scaled_body_size / 2, scaled_body_size / 2};
        DrawRectanglePro(body_rect, body_origin, m_death_rotation, body_color);
        
        // Calculate rotated gun position
        float gun_local_x = scaled_gun_offset_x * m_patrol_direction;
        float gun_local_y = scaled_gun_offset_y;
        
        float gun_rotated_x = cos(rotation_rad) * gun_local_x - sin(rotation_rad) * gun_local_y;
        float gun_rotated_y = sin(rotation_rad) * gun_local_x + cos(rotation_rad) * gun_local_y;
        
        float gun_world_x = draw_x + gun_rotated_x;
        float gun_world_y = draw_y + gun_rotated_y;
        
        Rectangle gun_rect = {
            gun_world_x,
            gun_world_y,
            scaled_gun_length,
            scaled_gun_width
        };
        Vector2 gun_origin = {0, scaled_gun_width / 2};
        DrawRectanglePro(gun_rect, gun_origin, m_death_rotation, gun_color);
        
        // Draw eyes (rotated positions)
        float left_eye_local_x = (scaled_eye_offset_x - scaled_eye_spacing / 2) * m_patrol_direction;
        float left_eye_local_y = scaled_eye_offset_y;
        float right_eye_local_x = (scaled_eye_offset_x + scaled_eye_spacing / 2) * m_patrol_direction;
        float right_eye_local_y = scaled_eye_offset_y;
        
        float left_eye_rotated_x = cos(rotation_rad) * left_eye_local_x - sin(rotation_rad) * left_eye_local_y;
        float left_eye_rotated_y = sin(rotation_rad) * left_eye_local_x + cos(rotation_rad) * left_eye_local_y;
        float right_eye_rotated_x = cos(rotation_rad) * right_eye_local_x - sin(rotation_rad) * right_eye_local_y;
        float right_eye_rotated_y = sin(rotation_rad) * right_eye_local_x + cos(rotation_rad) * right_eye_local_y;
        
        draw_circle(draw_x + left_eye_rotated_x, draw_y + left_eye_rotated_y, scaled_eye_size, eye_color);
        draw_circle(draw_x + right_eye_rotated_x, draw_y + right_eye_rotated_y, scaled_eye_size, eye_color);
        
        return;
    }
    
    // Normal drawing (alive)
    draw_rectangle(
        position.x - scaled_body_size / 2,
        position.y - scaled_body_size / 2,
        scaled_body_size,
        scaled_body_size,
        body_color
    );
    
    float gun_x = position.x + (scaled_gun_offset_x * m_patrol_direction);
    float gun_y = position.y + scaled_gun_offset_y;
    
    float gun_draw_x = (m_patrol_direction == 1) ? gun_x : gun_x - scaled_gun_length;
    
    draw_rectangle(
        gun_draw_x,
        gun_y - scaled_gun_width / 2,
        scaled_gun_length,
        scaled_gun_width,
        gun_color
    );
    
    float left_eye_x = position.x + (scaled_eye_offset_x - scaled_eye_spacing / 2) * m_patrol_direction;
    float right_eye_x = position.x + (scaled_eye_offset_x + scaled_eye_spacing / 2) * m_patrol_direction;
    float eyes_y = position.y + scaled_eye_offset_y;
    
    draw_circle(left_eye_x, eyes_y, scaled_eye_size, eye_color);
    draw_circle(right_eye_x, eyes_y, scaled_eye_size, eye_color);
}

void Enemy::draw_skull_marker() {
    // Floating animation
    m_skull_float_offset += get_frame_time() * 2.0f;
    float float_y = sin(m_skull_float_offset) * 5.0f;
    
    float marker_x = m_death_position.x;
    float marker_y = m_death_position.y - 40.0f + float_y;
    float skull_size = 20.0f;
    
    // Draw semi-transparent background circle
    draw_circle(marker_x, marker_y, skull_size * 0.8f, ColorAlpha(BLACK, 0.5f));
    
    // Draw skull (simplified skull shape)
    // Head
    draw_circle(marker_x, marker_y - 2.0f, skull_size * 0.5f, WHITE);
    
    // Eye sockets
    float eye_offset = skull_size * 0.2f;
    draw_circle(marker_x - eye_offset, marker_y - 4.0f, skull_size * 0.15f, BLACK);
    draw_circle(marker_x + eye_offset, marker_y - 4.0f, skull_size * 0.15f, BLACK);
    
    // Nose
    Vector2 nose_p1 = {marker_x - 3.0f, marker_y + 2.0f};
    Vector2 nose_p2 = {marker_x + 3.0f, marker_y + 2.0f};
    Vector2 nose_p3 = {marker_x, marker_y + 6.0f};
    DrawTriangle(nose_p1, nose_p2, nose_p3, BLACK);
    
    // Jaw/mouth area (teeth)
    draw_rectangle(marker_x - skull_size * 0.3f, marker_y + 8.0f, 
                  skull_size * 0.6f, 4.0f, WHITE);
    // Vertical lines for teeth
    draw_line(marker_x - skull_size * 0.2f, marker_y + 8.0f,
             marker_x - skull_size * 0.2f, marker_y + 12.0f, BLACK);
    draw_line(marker_x, marker_y + 8.0f,
             marker_x, marker_y + 12.0f, BLACK);
    draw_line(marker_x + skull_size * 0.2f, marker_y + 8.0f,
             marker_x + skull_size * 0.2f, marker_y + 12.0f, BLACK);
}
