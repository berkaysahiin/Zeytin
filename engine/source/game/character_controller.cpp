#include "game/character_controller.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/player_info.h"
#include "remote_logger/remote_logger.h"

void CharacterController::on_play_start() {
    map_key_bindings();
}

void CharacterController::on_play_update() {
    handle_input();
    apply_movement();
}

void CharacterController::map_key_bindings() {
    mapped_to_keycode.clear();
    
    mapped_to_keycode[MappedKey::MoveUp] = KEY_W;
    mapped_to_keycode[MappedKey::MoveDown] = KEY_S;
    mapped_to_keycode[MappedKey::MoveLeft] = KEY_A;
    mapped_to_keycode[MappedKey::MoveRight] = KEY_D;
    mapped_to_keycode[MappedKey::CommonAbility] = KEY_SPACE;
    mapped_to_keycode[MappedKey::RandomAbility] = KEY_E;

    const int player_index = Query::read<PlayerInfo>(this).index;
    
    switch (player_index) {
        case 1: 
            mapped_to_keycode[MappedKey::MoveUp] = KEY_UP;
            mapped_to_keycode[MappedKey::MoveDown] = KEY_DOWN;
            mapped_to_keycode[MappedKey::MoveLeft] = KEY_LEFT;
            mapped_to_keycode[MappedKey::MoveRight] = KEY_RIGHT;
            mapped_to_keycode[MappedKey::CommonAbility] = KEY_RIGHT_SHIFT;
            mapped_to_keycode[MappedKey::RandomAbility] = KEY_RIGHT_CONTROL;
            break;
            
        case 2: 
            mapped_to_keycode[MappedKey::MoveUp] = KEY_I;
            mapped_to_keycode[MappedKey::MoveDown] = KEY_K;
            mapped_to_keycode[MappedKey::MoveLeft] = KEY_J;
            mapped_to_keycode[MappedKey::MoveRight] = KEY_L;
            mapped_to_keycode[MappedKey::CommonAbility] = KEY_O;
            mapped_to_keycode[MappedKey::RandomAbility] = KEY_U;
            break;
        case 3: 

            mapped_to_keycode[MappedKey::MoveUp] = KEY_KP_8;
            mapped_to_keycode[MappedKey::MoveDown] = KEY_KP_2;
            mapped_to_keycode[MappedKey::MoveLeft] = KEY_KP_4;
            mapped_to_keycode[MappedKey::MoveRight] = KEY_KP_6;
            mapped_to_keycode[MappedKey::CommonAbility] = KEY_KP_0;
            mapped_to_keycode[MappedKey::RandomAbility] = KEY_KP_ENTER;
            break;
            
        default: 
            break;
    }
}

int CharacterController::get_keycode(MappedKey key) const {
    auto it = mapped_to_keycode.find(key);
    if (it != mapped_to_keycode.end()) {
        return it->second;
    }
    return KEY_NULL; 
}

void CharacterController::handle_input() {
    m_input_direction = {0.0f, 0.0f};
    
    if (is_key_down(get_keycode(MappedKey::MoveUp))) {
        m_input_direction.y -= 1.0f;
    }
    if (is_key_down(get_keycode(MappedKey::MoveDown))) {
        m_input_direction.y += 1.0f;
    }
    if (is_key_down(get_keycode(MappedKey::MoveLeft))) {
        m_input_direction.x -= 1.0f;
    }
    if (is_key_down(get_keycode(MappedKey::MoveRight))) {
        m_input_direction.x += 1.0f;
    }
    
    if (vector2_length(m_input_direction) > 0.0f) {
        m_input_direction = vector2_normalize(m_input_direction);
    }
}

void CharacterController::apply_movement() {
    auto& position = Query::get<Position>(this);
    auto& speed = Query::get<Speed>(this);
    
    if (vector2_length(m_input_direction) > 0.0f) {
        m_velocity.x += m_input_direction.x * acceleration * get_frame_time();
        m_velocity.y += m_input_direction.y * acceleration * get_frame_time();
        
        float current_speed = vector2_length(m_velocity);
        if (current_speed > speed.value) {
            m_velocity = vector2_scale(vector2_normalize(m_velocity), speed.value);
        }
    } 
    else {
        float friction_amount = friction * get_frame_time();;
        float current_speed = vector2_length(m_velocity);
        
        if (current_speed <= friction_amount) {
            m_velocity = {0.0f, 0.0f};
        } else {
            m_velocity = vector2_scale(m_velocity, 1.0f - friction_amount / current_speed);
        }
    }
    
    position.x += m_velocity.x * get_frame_time();
    position.y += m_velocity.y * get_frame_time();
}
