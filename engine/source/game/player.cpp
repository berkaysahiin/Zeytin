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
#include "remote_logger/remote_logger.h"
#include <algorithm>
#include <cmath>

void Player::on_init() {
    m_velocity = {0.0f, 0.0f};
    m_is_grounded = false;
    m_facing_direction = 1;
    m_jumps_remaining = max_jumps;
    m_has_diffuser = false;
    m_was_grounded_last_frame = false;
    m_just_jumped = false;
    m_target_scale = {1.0f, 1.0f};
    
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
    update_squash_stretch();  
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
    
    bool jump_pressed = is_key_down(KEY_SPACE) || is_key_down(KEY_UP) || is_key_down(KEY_W);
    
    if (jump_pressed && !m_jump_pressed_last_frame) {
        if (m_is_grounded || (enable_double_jump && m_jumps_remaining > 0)) {
            m_velocity.y = -jump_force;
            m_jumps_remaining--;
            m_is_grounded = false;
            m_just_jumped = true; 
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
    
    float old_x = position.x;
    position.x += m_velocity.x * delta;

	float left_bound = body_size / 2;
    float right_bound = VIRTUAL_WIDTH - body_size / 2;
    if (position.x < left_bound) {
        position.x = left_bound;
        m_velocity.x = 0;
    } else if (position.x > right_bound) {
        position.x = right_bound;
        m_velocity.x = 0;
    }
    
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
    
    float old_y = position.y;
    position.y += m_velocity.y * delta;
    
    for (::entity_id obstacle_id : obstacle_entities) {
        auto& obstacle_collider = Query::get<Collider>(obstacle_id);
        Rectangle obstacle_bounds = obstacle_collider.get_bounds();
        Rectangle player_bounds = player_collider.get_bounds();
        
        if (CheckCollisionRecs(player_bounds, obstacle_bounds)) {
            position.y = old_y;
            m_velocity.y = 0;
            m_jumps_remaining = max_jumps;
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
}

void Player::update_squash_stretch() {
	#if 0
    auto& scale = Query::get<Scale>(this);
    
    bool just_landed = !m_was_grounded_last_frame && m_is_grounded;
    
    if (just_landed) {
        m_target_scale.y = 1.0f - squash_amount; 
        m_target_scale.x = 1.0f + squash_amount;  
    }
    else if (m_just_jumped) {
        m_target_scale.y = 1.0f + stretch_amount;  // e.g., 1.3 (stretched up)
        m_target_scale.x = 1.0f - stretch_amount * 0.5f;  // e.g., 0.85 (narrower)
        m_just_jumped = false;
    }
    else {
        m_target_scale = {1.0f, 1.0f};
    }
    
    // Smoothly lerp current scale towards target scale
    float delta = get_frame_time();
    float lerp_factor = 1.0f - std::pow(0.001f, delta * squash_speed);
    
    scale.x += (m_target_scale.x - scale.x) * lerp_factor;
    scale.y += (m_target_scale.y - scale.y) * lerp_factor;
    
    // Clamp to reasonable values (prevent extreme scaling bugs)
    scale.x = std::clamp(scale.x, 0.3f, 2.0f);
    scale.y = std::clamp(scale.y, 0.3f, 2.0f);
    
    // Update state for next frame
    m_was_grounded_last_frame = m_is_grounded;
#endif
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
            float enemy_center_y = enemy_bounds.y + enemy_bounds.height / 2;
            
            if (player_bottom < enemy_center_y) {
                enemy.kill();
                m_velocity.y = -jump_force * 0.7f;
                m_jumps_remaining = std::min(m_jumps_remaining + 1, max_jumps);
                m_just_jumped = true;
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
            break;
        }
    }
}

void Player::draw_character() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float scaled_width = body_size * scale.x;
    float scaled_height = body_size * scale.y;
    
    draw_rectangle(
        position.x - scaled_width / 2,
        position.y - scaled_height / 2,
        scaled_width,
        scaled_height,
        body_color
    );
    
    float scaled_eye_size = eye_size * std::min(scale.x, scale.y);
    float scaled_eye_offset_x = eye_offset_x * scale.x;
    float scaled_eye_offset_y = eye_offset_y * scale.y;
    float scaled_eye_spacing = eye_spacing * scale.x;
    
    float left_eye_x = position.x - scaled_eye_spacing / 2 + (m_facing_direction * scaled_eye_offset_x);
    float right_eye_x = position.x + scaled_eye_spacing / 2 + (m_facing_direction * scaled_eye_offset_x);
    float eye_y = position.y + scaled_eye_offset_y;
    
    draw_circle(left_eye_x, eye_y, scaled_eye_size, eye_color);
    draw_circle(right_eye_x, eye_y, scaled_eye_size, eye_color);
    
    draw_christmas_hat();
}

void Player::draw_christmas_hat() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float scaled_hat_width = hat_base_width * scale.x;
    float scaled_hat_height = hat_height * scale.y;
    float scaled_trim_height = hat_trim_height * scale.y;
    float scaled_pom_radius = hat_pom_radius * std::min(scale.x, scale.y);
    
    float hat_x = position.x;
    float hat_base_y = position.y + (hat_offset_y * scale.y);
    float hat_tip_y = hat_base_y - scaled_hat_height;
    
    float hat_left_x = hat_x - scaled_hat_width / 2;
    float hat_right_x = hat_x + scaled_hat_width / 2;
    float hat_tip_x = hat_x + (scaled_hat_width * 0.15f);
    
    draw_triangle(
        {hat_left_x, hat_base_y},
        {hat_right_x, hat_base_y},
        {hat_tip_x, hat_tip_y},
        hat_color
    );
    
    draw_rectangle(
        hat_left_x,
        hat_base_y,
        scaled_hat_width,
        scaled_trim_height,
        hat_trim_color
    );
    
    draw_circle(hat_tip_x, hat_tip_y, scaled_pom_radius, hat_pom_color);
}

void Player::draw_diffuser_icon() {
    auto& position = Query::get<Position>(this);
    auto& scale = Query::get<Scale>(this);
    
    float icon_size = 20.0f;
    float scaled_icon_size = icon_size * scale.x;
    
    float icon_x = position.x;
    float icon_y = position.y - (body_size * scale.y) / 2 - scaled_icon_size - 10;
    
    Color diffuser_color = {255, 215, 0, 255};
    
    draw_rectangle(
        icon_x - scaled_icon_size / 4,
        icon_y - scaled_icon_size / 2,
        scaled_icon_size / 2,
        scaled_icon_size,
        diffuser_color
    );
    
    draw_circle(icon_x, icon_y, scaled_icon_size * 0.6f, ColorAlpha(diffuser_color, 0.4f));
}
