#include "game/player.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "entity/entity.h"
#include "game/collider.h"
#include "game/obstacle.h"
#include "game/position.h"
#include "game/scale.h"
#include "game/enemy.h"

void Player::on_init() {
    m_velocity = {0.0f, 0.0f};
    m_is_grounded = false;
    m_facing_direction = 1;
    m_jumps_remaining = max_jumps;
    
    body_color = {88, 83, 86, 255};
    eye_color = WHITE;
}

void Player::on_update() {
    draw_character();
}

void Player::on_play_update() {
    handle_input();
    apply_physics();
    check_ground();
    check_enemy_collision();
}

void Player::handle_input() {
    float horizontal = 0.0f;
    
    if (is_key_down(KEY_A) || is_key_down(KEY_LEFT)) {
        horizontal -= 1.0f;
    }
    if (is_key_down(KEY_D) || is_key_down(KEY_RIGHT)) {
        horizontal += 1.0f;
    }
    
    if (horizontal < 0) {
        m_facing_direction = -1;
    } else if (horizontal > 0) {
        m_facing_direction = 1;
    }
    
    m_velocity.x = horizontal * move_speed;
    
    bool jump_pressed = is_key_down(KEY_SPACE) || is_key_down(KEY_UP);
    
    if (jump_pressed && !m_jump_pressed_last_frame) {
        if (m_is_grounded || (enable_double_jump && m_jumps_remaining > 0)) {
            m_velocity.y = -jump_force;
            m_jumps_remaining--;
            m_is_grounded = false;
        }
    }
    
    m_jump_pressed_last_frame = jump_pressed;
}

void Player::apply_physics() {
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

void Player::check_ground() {
    auto& position = Query::get<Position>(this);
    auto& player_collider = Query::get<Collider>(this);
    
    float ground_level = VIRTUAL_HEIGHT - body_size / 2;
    bool landed = false;
    
    Rectangle player_bounds = player_collider.get_bounds();
    float player_bottom = position.y + player_collider.height / 2;
    
    auto obstacle_entities = Query::find_all_with<Collider, Obstacle>();
    
    for (::entity_id obstacle_id : obstacle_entities) {
        auto& obstacle_collider = Query::get<Collider>(obstacle_id);
        Rectangle obstacle_bounds = obstacle_collider.get_bounds();
        
        if (m_velocity.y >= 0 &&  // Falling down
            player_bounds.x + player_bounds.width > obstacle_bounds.x &&
            player_bounds.x < obstacle_bounds.x + obstacle_bounds.width &&
            player_bottom >= obstacle_bounds.y &&
            player_bottom <= obstacle_bounds.y + 10) {  
            
            position.y = obstacle_bounds.y - player_collider.height / 2;
            m_velocity.y = 0.0f;
            landed = true;
            m_jumps_remaining = max_jumps;
            break;
        }
    }
    
    if (!landed && position.y >= ground_level) {
        position.y = ground_level;
        m_velocity.y = 0.0f;
        landed = true;
        m_jumps_remaining = max_jumps;
    }
    
    m_is_grounded = landed;
}

void Player::check_enemy_collision() {
    auto& player_pos = Query::get<Position>(this);
    auto& player_collider = Query::get<Collider>(this);
    
    Rectangle player_bounds = player_collider.get_bounds();
    float player_bottom = player_pos.y + player_collider.height / 2;
    
    auto enemy_entities = Query::find_all_with<Enemy, Position, Collider>();
    
    for (::entity_id enemy_id : enemy_entities) {
        auto& enemy = Query::get<Enemy>(enemy_id);
        
        if (enemy.is_dead()) continue;
        
        auto& enemy_pos = Query::get<Position>(enemy_id);
        auto& enemy_collider = Query::get<Collider>(enemy_id);
        
        Rectangle enemy_bounds = enemy_collider.get_bounds();
        float enemy_top = enemy_pos.y - enemy_collider.height / 2;
        
        if (check_collision_recs(player_bounds, enemy_bounds)) {
            if (m_velocity.y > 0 && player_bottom <= enemy_top + 15.0f) {
                enemy.kill();
                m_velocity.y = -jump_force * 0.5f;
            }
        }
    }
}

void Player::draw_character() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float scaled_body_size = body_size * scale.x;
    float scaled_eye_size = eye_size * scale.x;
    float scaled_eye_offset_x = eye_offset_x * scale.x;
    float scaled_eye_offset_y = eye_offset_y * scale.y;
    float scaled_eye_spacing = eye_spacing * scale.x;
    
    draw_rectangle(
        position.x - scaled_body_size / 2,
        position.y - scaled_body_size / 2,
        scaled_body_size,
        scaled_body_size,
        body_color
    );
    
    float left_eye_x = position.x + (scaled_eye_offset_x - scaled_eye_spacing / 2) * m_facing_direction;
    float right_eye_x = position.x + (scaled_eye_offset_x + scaled_eye_spacing / 2) * m_facing_direction;
    float eyes_y = position.y + scaled_eye_offset_y;
    
    draw_circle(left_eye_x, eyes_y, scaled_eye_size, eye_color);
    draw_circle(right_eye_x, eyes_y, scaled_eye_size, eye_color);
}
