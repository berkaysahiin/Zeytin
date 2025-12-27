#include "game/player.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "entity/entity.h"
#include "game/collider.h"
#include "game/obstacle.h"
#include "game/position.h"
#include "game/scale.h"
#include "game/enemy.h"
#include "game/bullet.h"
#include "game/end_game.h"
#include <algorithm>

void Player::on_init() {
    m_velocity = {0.0f, 0.0f};
    m_is_grounded = false;
    m_facing_direction = 1;
    m_jumps_remaining = max_jumps;
    m_has_diffuser = false;
    
    body_color = {88, 83, 86, 255};
    eye_color = WHITE;
}

void Player::on_update() {
    draw_character();
    
    if (m_has_diffuser) {
        draw_diffuser_icon();
    }
}

void Player::on_play_update() {
    auto end_game_opt = Query::try_find_first<EndGame>();
    if (end_game_opt && end_game_opt->get().is_game_over()) {
        return;
    }
    
    handle_input();
    apply_physics();
    check_ground();
    check_enemy_collision();
    check_bullet_collision();
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
    auto& player_collider = Query::get<Collider>(this);
    float delta = get_frame_time();
    
    if (!m_is_grounded) {
        m_velocity.y += gravity * delta;
        if (m_velocity.y > max_fall_speed) {
            m_velocity.y = max_fall_speed;
        }
    }
    
    // Move horizontally and check collision
    float old_x = position.x;
    position.x += m_velocity.x * delta;
    
    auto obstacle_entities = Query::find_all_with<Collider, Obstacle>();
    for (::entity_id obstacle_id : obstacle_entities) {
        auto& obstacle_collider = Query::get<Collider>(obstacle_id);
        Rectangle obstacle_bounds = obstacle_collider.get_bounds();
        Rectangle player_bounds = player_collider.get_bounds();
        
        if (CheckCollisionRecs(player_bounds, obstacle_bounds)) {
            position.x = old_x;
            m_velocity.x = 0;
            break;
        }
    }
    
    // Move vertically and check collision
    float old_y = position.y;
    position.y += m_velocity.y * delta;
    
    for (::entity_id obstacle_id : obstacle_entities) {
        auto& obstacle_collider = Query::get<Collider>(obstacle_id);
        Rectangle obstacle_bounds = obstacle_collider.get_bounds();
        Rectangle player_bounds = player_collider.get_bounds();
        
        if (CheckCollisionRecs(player_bounds, obstacle_bounds)) {
            position.y = old_y;
            m_velocity.y = 0;
            break;
        }
    }
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
        
        if (m_velocity.y >= 0 &&
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

	if(m_is_grounded) {
		m_jumps_remaining = max_jumps;
	}
}

void Player::check_enemy_collision() {
    auto& player_pos = Query::get<Position>(this);
    auto& player_collider = Query::get<Collider>(this);
    
    Rectangle player_bounds = player_collider.get_bounds();
    float player_bottom = player_pos.y + player_collider.height / 2;
    
    auto enemy_entities = Query::find_all_with<Enemy, Collider>();
    
    for (::entity_id enemy_id : enemy_entities) {
        auto& enemy = Query::get<Enemy>(enemy_id);
        
        if (enemy.is_dead()) continue;
        
        auto& enemy_collider = Query::get<Collider>(enemy_id);
        Rectangle enemy_bounds = enemy_collider.get_bounds();
        
        if (CheckCollisionRecs(player_bounds, enemy_bounds)) {
            float enemy_top = enemy_bounds.y;
            
            if (m_velocity.y > 0 && player_bottom <= enemy_top + 15) {
                enemy.kill();
                m_velocity.y = -jump_force * 0.7f;
				m_jumps_remaining = std::min(m_jumps_remaining + 1, max_jumps);
                continue;
            }
            
            auto end_game_opt = Query::try_find_first<EndGame>();
            if (end_game_opt) {
                end_game_opt->get().trigger_game_over("Hit by enemy!");
            }
            break;
        }
    }
}

void Player::check_bullet_collision() {
    auto& player_collider = Query::get<Collider>(this);
    Rectangle player_bounds = player_collider.get_bounds();
    
    auto bullet_entities = Query::find_all_with<Bullet, Collider>();
    for (::entity_id bullet_id : bullet_entities) {
        auto& bullet_collider = Query::get<Collider>(bullet_id);
        Rectangle bullet_bounds = bullet_collider.get_bounds();
        
        if (CheckCollisionRecs(player_bounds, bullet_bounds)) {
            auto end_game_opt = Query::try_find_first<EndGame>();
            if (end_game_opt) {
                end_game_opt->get().trigger_game_over("Hit by enemy bullet!");
            }
            
            Query::remove_entity(bullet_id);
            break;
        }
    }
}

void Player::draw_character() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float scaled_size = body_size * scale.x;
    float scaled_eye_size = eye_size * scale.x;
    
    draw_rectangle(
        position.x - scaled_size / 2,
        position.y - scaled_size / 2,
        scaled_size,
        scaled_size,
        body_color
    );
    
    float eye_y = position.y + eye_offset_y * scale.y;
    
    if (m_facing_direction > 0) {
        float left_eye_x = position.x + (eye_offset_x - eye_spacing / 2) * scale.x;
        float right_eye_x = position.x + (eye_offset_x + eye_spacing / 2) * scale.x;
        
        draw_circle(left_eye_x, eye_y, scaled_eye_size, eye_color);
        draw_circle(right_eye_x, eye_y, scaled_eye_size, eye_color);
    } else {
        float left_eye_x = position.x - (eye_offset_x + eye_spacing / 2) * scale.x;
        float right_eye_x = position.x - (eye_offset_x - eye_spacing / 2) * scale.x;
        
        draw_circle(left_eye_x, eye_y, scaled_eye_size, eye_color);
        draw_circle(right_eye_x, eye_y, scaled_eye_size, eye_color);
    }
}

void Player::draw_diffuser_icon() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float icon_size = 15.0f;
    float offset_y = -body_size * 0.7f;
    
    float icon_x = position.x;
    float icon_y = position.y + offset_y;
    
    Color icon_color = {0, 255, 255, 255};
    
    draw_rectangle(
        icon_x - icon_size / 4,
        icon_y - icon_size / 2,
        icon_size / 2,
        icon_size,
        icon_color
    );
    
    draw_rectangle(
        icon_x - icon_size / 6,
        icon_y + icon_size / 4,
        icon_size / 3,
        icon_size / 2,
        icon_color
    );
    
    draw_circle(icon_x, icon_y, icon_size * 0.6f, 
               ColorAlpha(icon_color, 0.3f));
}
