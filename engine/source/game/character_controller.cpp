#include "game/character_controller.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/player_info.h"
#include "remote_logger/remote_logger.h"

#include "game/wall.h"

void CharacterController::on_play_start() {
    auto& collider = Query::get<Collider>(this);
    collider.m_callback = [&](Collider& other) {
        const bool is_wall = Query::has<Wall>(other.entity_id);
        if(is_wall) {
            bounce_from_boundries(other);
        }

        const bool is_player = Query::has<PlayerInfo>(other.entity_id);
        if(is_player) {
            push_each_other(other);
        }
    };

    map_key_bindings();
}

void CharacterController::on_play_update() {
    handle_input();
    apply_movement();
}

void CharacterController::map_key_bindings() {
    mapped_to_keycode.clear();
    
    mapped_to_keycode[MappedKey::MoveUp] = KEY_UP;
    mapped_to_keycode[MappedKey::MoveDown] = KEY_DOWN;
    mapped_to_keycode[MappedKey::MoveLeft] = KEY_LEFT;
    mapped_to_keycode[MappedKey::MoveRight] = KEY_RIGHT;
    mapped_to_keycode[MappedKey::CommonAbility] = KEY_RIGHT_SHIFT;
    mapped_to_keycode[MappedKey::RandomAbility] = KEY_RIGHT_CONTROL;

    const int player_index = Query::read<PlayerInfo>(this).index;
    
    switch (player_index) {
        case 1: 
            mapped_to_keycode[MappedKey::MoveUp] = KEY_W;
            mapped_to_keycode[MappedKey::MoveDown] = KEY_S;
            mapped_to_keycode[MappedKey::MoveLeft] = KEY_A;
            mapped_to_keycode[MappedKey::MoveRight] = KEY_D;
            mapped_to_keycode[MappedKey::CommonAbility] = KEY_SPACE;
            mapped_to_keycode[MappedKey::RandomAbility] = KEY_E;
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

void CharacterController::bounce_from_boundries(Collider& other) {
    auto [position, collider] = Query::get<Position, Collider>(this);
    auto& other_position = Query::get<Position>(other.entity_id);

    if(other.m_collider_type == 1) { // Rectangle
        Rectangle rect = other.get_rectangle();

        float closest_x = fmaxf(rect.x, fminf(position.x, rect.x + rect.width));
        float closest_y = fmaxf(rect.y, fminf(position.y, rect.y + rect.height));

        Vector2 normal = {
            position.x - closest_x,
            position.y - closest_y
        };

        bool is_corner = (closest_x != position.x && closest_y != position.y);

        if (!is_corner && Vector2Length(normal) > 0) {
            if (fabsf(normal.x) < fabsf(normal.y)) {
                normal.x = 0;
            } else {
                normal.y = 0;
            }
        }

        if (Vector2Length(normal) > 0) {
            normal = Vector2Normalize(normal);
        } else {
            normal = {0, -1};
        }

        float penetration = collider.m_radius - Vector2Distance({position.x, position.y}, {closest_x, closest_y});
        if (penetration > 0) {
            position.x += normal.x * penetration;
            position.y += normal.y * penetration;
        }

        float dot_product = m_velocity.x * normal.x + m_velocity.y * normal.y;
        if (dot_product < 0) {
            m_velocity.x = m_velocity.x - 2 * dot_product * normal.x;
            m_velocity.y = m_velocity.y - 2 * dot_product * normal.y;
        }
    }
}

void CharacterController::push_each_other(Collider& other) {
    auto& my_position = Query::get<Position>(this);
    auto& other_position = Query::get<Position>(other.entity_id);
    auto& my_collider = Query::get<Collider>(this);

    Vector2 push_direction = {
        my_position.x - other_position.x,
        my_position.y - other_position.y
    };

    float min_distance = my_collider.get_radius() + other.get_radius();
    float current_distance = vector2_length(push_direction);
    float overlap = min_distance - current_distance;

    if (current_distance < 0.001f) {
        push_direction.x = GetRandomValue(-100, 100) / 100.0f;
        push_direction.y = GetRandomValue(-100, 100) / 100.0f;
        push_direction = vector2_normalize(push_direction);
        current_distance = 0.001f;  
    } else {
        push_direction = vector2_scale(push_direction, 1.0f/current_distance);  
    }

    if (Query::has<CharacterController>(other.entity_id)) {
        auto& other_controller = Query::get<CharacterController>(other.entity_id);

        if (overlap > 0) {
            Vector2 separation = vector2_scale(push_direction, overlap * 0.5f);

            my_position.x += separation.x;
            my_position.y += separation.y;
            other_position.x -= separation.x;
            other_position.y -= separation.y;
        }

        const float push_strength = 600.0f;  
        const float velocity_retention = 0.2f;  

        Vector2 impulse = vector2_scale(push_direction, push_strength);

        Vector2 my_new_velocity = {
            m_velocity.x * velocity_retention + impulse.x,
            m_velocity.y * velocity_retention + impulse.y
        };

        Vector2 other_new_velocity = {
            other_controller.m_velocity.x * velocity_retention - impulse.x,
            other_controller.m_velocity.y * velocity_retention - impulse.y
        };

        m_velocity = my_new_velocity;
        other_controller.m_velocity = other_new_velocity;
    }
}
