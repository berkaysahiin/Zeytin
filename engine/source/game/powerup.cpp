#include "game/powerup.h"
#include <cstdint>
#include "game/player_info.h"
#include "game/speed.h"
#include "game/scale.h"
#include "game/character_controller.h"
#include "game/zone.h"
#include "game/particle_system.h"

#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "raylib.h"
#include "remote_logger/remote_logger.h"

void PowerUp::on_post_init() {
    auto& collider = Query::get<Collider>(this);
    collider.m_callback = [&](Collider& other) {
        handle_player_collision(other);
    };

    if(m_type == Type::TELEPORTER) {
        m_lifetime *= 3;
    }
}

void PowerUp::handle_player_collision(Collider& other) {
    if(auto player_info = Query::try_get<PlayerInfo>(other.entity_id)) {
        int player_index = player_info->get().index;

         if (m_type == Type::TELEPORTER) {
            if (m_cooldown_remaining <= 0 && !m_is_exit_only) {
                teleport_player(other.entity_id);
                return; 
            }
            return;
        }

        apply_effect_to_player(player_index);
        consume();
    }
}

void PowerUp::apply_effect_to_player(int player_index) {
    auto players = Query::find_where<PlayerInfo>([player_index](PlayerInfo& info) {
        return info.index == player_index;
    });
    
    if (players.empty()) return;
    
    PlayerInfo& player = players[0].get();
    auto player_id = player.entity_id;
    m_player_id = player.entity_id;
    
    switch (m_type) {
        case Type::SPEED_BOOST: {
            if (auto speed = Query::try_get<Speed>(player_id)) {
                float original = speed->get().value;
                speed->get().value *= m_power_multiplier;
            }
            break;
        }
        
        case Type::SUPER_SIZE: {
            if (auto scale = Query::try_get<Scale>(player_id)) {
                scale->get().x *= m_power_multiplier;
                scale->get().y *= m_power_multiplier;
                
                if (auto collider = Query::try_get<Collider>(player_id)) {
                    collider->get().m_radius *= m_power_multiplier;
                }
            }
            break;
        }
        
        case Type::SHRINK: {
            if (auto scale = Query::try_get<Scale>(player_id)) {

                float shrink_factor = 1.0f / m_power_multiplier;
                scale->get().x *= shrink_factor;
                scale->get().y *= shrink_factor;
                
                if (auto collider = Query::try_get<Collider>(player_id)) {
                    collider->get().m_radius *= shrink_factor;
                }
            }
            break;
        }
        
        case Type::ZONE_SLOW: {
              for(auto& zone_ref : Query::find_all<Zone>()) {
                  zone_ref.get().vanish_after_secs *= m_power_multiplier;
              }
            break;
        }

        case Type::SHIELD: {
            player.color = ColorAlpha(player.color, 0.1);  
            player.shield = true;
        }
    }
}

void PowerUp::on_update() {
    if(m_used) {
        return;
    }

    const auto& position = Query::read<Position>(this);
    
    float time = (float)get_time();
    float pulse = (sinf(time * 4.0f) * 0.2f + 0.8f);
    
    switch (m_type) {
        case Type::SPEED_BOOST:
            draw_speed_boost(position, pulse, time);
            break;
            
        case Type::SUPER_SIZE:
            draw_super_size(position, pulse, time);
            break;
            
        case Type::SHRINK:
            draw_shrink(position, pulse, time);
            break;
            
        case Type::ZONE_SLOW:
            draw_zone_slow(position, pulse, time);
            break;
            
        case Type::SHIELD:
            draw_shield(position, pulse, time);
            break;
        case Type::TELEPORTER:
            draw_teleporter(position, pulse, time);
    }
    
    float base_radius = 20.0f;
    if (m_since_spawn > m_lifetime * 0.7f) {
        draw_expiration_warning(position, time, base_radius);
    }
}

void PowerUp::draw_speed_boost(const Position& position, float pulse, float time) {
    float base_radius = 20.0f;
    float core_radius = 12.0f;
    float rotation = time * 90.0f; 
    
    Color outer_color = BLUE;
    Color inner_color = SKYBLUE;
    
    draw_circle(position.x, position.y, base_radius * pulse, ColorAlpha(outer_color, 0.3f));
    draw_circle(position.x, position.y, base_radius * 0.8f, outer_color);
    draw_circle(position.x, position.y, core_radius, inner_color);
    
    for (int i = 0; i < 3; i++) {
        float angle = rotation + i * 120.0f;
        float rad = angle * DEG2RAD;
        float streakLength = base_radius * 1.5f;
        
        Vector2 start = {
            position.x + cosf(rad) * core_radius,
            position.y + sinf(rad) * core_radius
        };
        
        Vector2 end = {
            position.x + cosf(rad) * streakLength,
            position.y + sinf(rad) * streakLength
        };
        
        float thickness = 3.0f * (0.5f + 0.5f * sinf(time * 5.0f + i));
        DrawLineEx(start, end, thickness, WHITE);
    }
    
    float bolt_scale = 7.0f;
    Vector2 bolt[] = {
        {position.x - 0.2f * bolt_scale, position.y - 0.6f * bolt_scale},
        {position.x + 0.3f * bolt_scale, position.y - 0.1f * bolt_scale},
        {position.x,                     position.y - 0.0f * bolt_scale},
        {position.x + 0.2f * bolt_scale, position.y + 0.6f * bolt_scale},
        {position.x - 0.3f * bolt_scale, position.y + 0.1f * bolt_scale},
        {position.x,                     position.y + 0.0f * bolt_scale},
    };
    
    for (int i = 0; i < 3; i++) {
        DrawLineEx(bolt[i*2], bolt[i*2+1], 2.0f, WHITE);
    }
}

void PowerUp::draw_super_size(const Position& position, float pulse, float time) {
    float base_radius = 20.0f;
    float core_radius = 12.0f;
    
    Color outer_color = GREEN;
    Color inner_color = LIME;
    
    draw_circle(position.x, position.y, base_radius * pulse, ColorAlpha(outer_color, 0.3f));
    draw_circle(position.x, position.y, base_radius * 0.9f, outer_color);
    draw_circle(position.x, position.y, core_radius, inner_color);
    
    float ring_pulse1 = fmodf(time, 1.0f);
    float ring_pulse2 = fmodf(time + 0.5f, 1.0f);
    
    draw_circle_lines(position.x, position.y, base_radius * (1.0f + ring_pulse1 * 0.8f), 
                     ColorAlpha(LIME, 1.0f - ring_pulse1));
    draw_circle_lines(position.x, position.y, base_radius * (1.0f + ring_pulse2 * 0.8f), 
                     ColorAlpha(LIME, 1.0f - ring_pulse2));
    
    float arrow_size = base_radius * 0.5f;
    for (int i = 0; i < 4; i++) {
        float angle = i * 90.0f * DEG2RAD;
        
        Vector2 start = {
            position.x + cosf(angle) * (core_radius * 0.6f),
            position.y + sinf(angle) * (core_radius * 0.6f)
        };
        
        Vector2 end = {
            position.x + cosf(angle) * arrow_size,
            position.y + sinf(angle) * arrow_size
        };
        
        DrawLineEx(start, end, 2.0f, WHITE);
        
        float head_size = 3.0f;
        float head_angle1 = angle + 135.0f * DEG2RAD;
        float head_angle2 = angle - 135.0f * DEG2RAD;
        
        Vector2 head1 = {
            end.x + cosf(head_angle1) * head_size,
            end.y + sinf(head_angle1) * head_size
        };
        
        Vector2 head2 = {
            end.x + cosf(head_angle2) * head_size,
            end.y + sinf(head_angle2) * head_size
        };
        
        DrawLineEx(end, head1, 2.0f, WHITE);
        DrawLineEx(end, head2, 2.0f, WHITE);
    }
}

void PowerUp::draw_shrink(const Position& position, float pulse, float time) {
    float base_radius = 20.0f;
    float core_radius = 12.0f;
    
    Color outer_color = GOLD;
    Color inner_color = YELLOW;
    
    draw_circle(position.x, position.y, base_radius * pulse, ColorAlpha(outer_color, 0.3f));
    draw_circle(position.x, position.y, base_radius * 0.9f, outer_color);
    draw_circle(position.x, position.y, core_radius, inner_color);
    
    float ring_pulse1 = 1.0f - fmodf(time, 1.0f);
    float ring_pulse2 = 1.0f - fmodf(time + 0.5f, 1.0f);
    
    draw_circle_lines(position.x, position.y, base_radius * (0.4f + ring_pulse1 * 0.6f), 
                    ColorAlpha(WHITE, ring_pulse1));
    draw_circle_lines(position.x, position.y, base_radius * (0.4f + ring_pulse2 * 0.6f), 
                    ColorAlpha(WHITE, ring_pulse2));
    
    float arrow_size = base_radius * 0.5f;
    for (int i = 0; i < 4; i++) {
        float angle = i * 90.0f * DEG2RAD;
        
        Vector2 start = {
            position.x + cosf(angle) * arrow_size,
            position.y + sinf(angle) * arrow_size
        };
        
        Vector2 end = {
            position.x + cosf(angle) * (core_radius * 0.6f),
            position.y + sinf(angle) * (core_radius * 0.6f)
        };
        
        DrawLineEx(start, end, 2.0f, WHITE);
        
        float head_size = 3.0f;
        float head_angle1 = angle + 45.0f * DEG2RAD;
        float head_angle2 = angle - 45.0f * DEG2RAD;
        
        Vector2 head1 = {
            end.x + cosf(head_angle1) * head_size,
            end.y + sinf(head_angle1) * head_size
        };
        
        Vector2 head2 = {
            end.x + cosf(head_angle2) * head_size,
            end.y + sinf(head_angle2) * head_size
        };
        
        DrawLineEx(end, head1, 2.0f, WHITE);
        DrawLineEx(end, head2, 2.0f, WHITE);
    }
    
    float minus_width = 8.0f;
    draw_line(position.x - minus_width/2, position.y, position.x + minus_width/2, position.y, WHITE);
}

void PowerUp::draw_zone_slow(const Position& position, float pulse, float time) {
    float base_radius = 20.0f;
    float core_radius = 12.0f;
    
    Color outer_color = PURPLE;
    Color inner_color = VIOLET;
    
    draw_circle(position.x, position.y, base_radius * pulse, ColorAlpha(outer_color, 0.3f));
    draw_circle(position.x, position.y, base_radius * 0.9f, outer_color);
    draw_circle(position.x, position.y, core_radius, inner_color);
    
    float slow_rotation = time * 30.0f; 
    float hand_length1 = core_radius * 0.8f;
    float hand_length2 = core_radius * 0.6f;

    Vector2 posv = {
        position.x,
        position.y
    };
    
    Vector2 hour_hand = {
        position.x + cosf(slow_rotation * DEG2RAD) * hand_length2,
        position.y + sinf(slow_rotation * DEG2RAD) * hand_length2
    };
    DrawLineEx(posv, hour_hand, 2.5f, WHITE);
    
    Vector2 minute_hand = {
        position.x + cosf((slow_rotation * 12) * DEG2RAD) * hand_length1,
        position.y + sinf((slow_rotation * 12) * DEG2RAD) * hand_length1
    };
    DrawLineEx(posv, minute_hand, 1.5f, WHITE);
    
    for (int i = 0; i < 4; i++) {
        float angle = i * 90.0f * DEG2RAD;
        Vector2 pos = {
            position.x + cosf(angle) * base_radius * 0.7f,
            position.y + sinf(angle) * base_radius * 0.7f
        };
        draw_circle(pos.x, pos.y, 2.0f, WHITE);
    }
    
    float outer_angle = 180.0f;
    float start_angle = 180.0f;
    float end_angle = start_angle + outer_angle;
    
    Color slow_color = ColorAlpha(SKYBLUE, 0.7f);
    DrawCircleSector(posv, base_radius * 0.4f, start_angle, end_angle, 12, slow_color);
}

void PowerUp::draw_shield(const Position& position, float pulse, float time) {
    float base_radius = 20.0f;
    float core_radius = 12.0f;
    
    Color outer_color = DARKPURPLE;
    Color inner_color = PURPLE;
    Color shield_color = SKYBLUE;
    
    draw_circle(position.x, position.y, base_radius * pulse, ColorAlpha(outer_color, 0.3f));
    draw_circle(position.x, position.y, base_radius * 0.9f, outer_color);
    draw_circle(position.x, position.y, core_radius, inner_color);
    
    int num_segments = 8;
    float segment_angle = 360.0f / num_segments;
    float rotation = time * 45.0f; 
    
    for (int i = 0; i < num_segments; i++) {
        float start_angle = rotation + i * segment_angle;
        float end_angle = start_angle + segment_angle * 0.7f; 
        
        DrawCircleSector({position.x, position.y}, 
                         base_radius * 1.3f, 
                         start_angle, 
                         end_angle, 
                         6, 
                         ColorAlpha(shield_color, 0.8f));
    }
    
    float shield_width = 8.0f;
    float shield_height = 10.0f;
    
    Vector2 shield_top = {position.x, position.y - shield_height/2};
    Vector2 shield_bottom = {position.x, position.y + shield_height/2};
    Vector2 shield_left = {position.x - shield_width/2, position.y - shield_height/4};
    Vector2 shield_right = {position.x + shield_width/2, position.y - shield_height/4};
    
    DrawTriangle(shield_top, shield_left, shield_right, WHITE);
    DrawTriangle(shield_bottom, shield_left, shield_right, WHITE);
}

void PowerUp::draw_expiration_warning(const Position& position, float time, float base_radius) {
    if (fmodf(time * 5.0f, 1.0f) > 0.5f) {
        draw_circle_lines(position.x, position.y, base_radius * 1.2f, RED);
        
        float remaining_pct = 1.0f - (m_since_spawn / m_lifetime);
        float arc_angle = 360.0f * remaining_pct;
        
        DrawCircleSector(
            {position.x, position.y},
            base_radius * 1.4f,
            0,
            arc_angle,
            20, 
            ColorAlpha(RED, 0.5f)
        );
    }
}

void PowerUp::on_play_update() {
    if(!m_used) { // time if not used
        m_since_spawn += get_frame_time();
        
        if (m_since_spawn >= m_lifetime) {
            Query::remove_entity(entity_id);
        }
    }
    else { 
        m_since_used += get_frame_time();
        if(!Query::entity_exists(m_player_id)) {
            log_error() << "Entity with id doesnot exist: " << m_player_id << std::endl;
        }
        if(m_since_used >= m_duration)  { // revert and say goodbye
            switch(m_type) {
                case Type::SPEED_BOOST: {
                    if(auto speed_opt = Query::try_get<Speed>(m_player_id)) {
                        auto& speed = speed_opt->get();
                        speed.value /= m_power_multiplier;
                    }
                    break;
                }
                case Type::SUPER_SIZE: {
                    auto& scale = Query::get<Scale>(m_player_id);
                    scale.x = 1;
                    scale.y = 1;
                    break;
                }                       
                case Type::SHRINK: {
                    auto& scale = Query::get<Scale>(m_player_id);
                    scale.x = 1;
                    scale.y = 1;
                    break;
                }                       
                case Type::SHIELD: {
                    auto& info = Query::get<PlayerInfo>(m_player_id);
                    info.color = ColorAlpha(info.color, 1);  
                    info.shield = false;
                    break;
                }

            }

            Query::remove_entity(entity_id);
        }
    }
}

void PowerUp::consume() {
    auto& collider = Query::get<Collider>(this);
    collider.set_enable(false);
    m_used = true;
}

void PowerUp::draw_teleporter(const Position& position, float pulse, float time) {
    float base_radius = 25.0f; 
    float core_radius = 15.0f;

    Color outer_color = BLACK;
    Color inner_color = BLACK;

    DrawCircleGradient(position.x, position.y, base_radius * 1.2f * pulse,
                       ColorAlpha(outer_color, 0.9f), ColorAlpha(BLACK, 0.4f));

    const int num_rings = 3;
    for (int i = 0; i < num_rings; i++) {
        float ring_radius = base_radius * (0.5f + 0.3f * i);
        float ring_rotation = time * (50.0f + i * 20.0f);
        float thickness = 2.0f + i * 0.5f;

        for (int j = 0; j < 4; j++) {
            float start_angle = ring_rotation + j * 90.0f;
            float end_angle = start_angle + 60.0f;

            DrawRing(
                (Vector2){position.x, position.y},
                ring_radius - thickness/2,
                ring_radius + thickness/2,
                start_angle,
                end_angle,
                0,
                ColorAlpha(WHITE, 0.7f - 0.15f * i)
            );
        }
    }

    const int num_particles = 12;
    float swirl_speed = time * 2.0f;

    for (int i = 0; i < num_particles; i++) {
        float angle = swirl_speed + i * (360.0f / num_particles);
        float distance = core_radius * (0.3f + 0.7f * (sinf(time * 3.0f + i) * 0.5f + 0.5f));

        float x = position.x + cosf(angle * DEG2RAD) * distance;
        float y = position.y + sinf(angle * DEG2RAD) * distance;

        float particle_size = 2.0f + sinf(time * 5.0f + i) * 1.5f;
        Color particle_color = ColorAlpha(WHITE, 0.7f + 0.3f * sinf(time * 2.0f + i));

        DrawCircleV((Vector2){x, y}, particle_size, particle_color);

        if (i > 0) {
            float prev_angle = swirl_speed + (i-1) * (360.0f / num_particles);
            float prev_distance = core_radius * (0.3f + 0.7f * (sinf(time * 3.0f + (i-1)) * 0.5f + 0.5f));

            float prev_x = position.x + cosf(prev_angle * DEG2RAD) * prev_distance;
            float prev_y = position.y + sinf(prev_angle * DEG2RAD) * prev_distance;

            DrawLineEx(
                (Vector2){prev_x, prev_y},
                (Vector2){x, y},
                1.0f,
                ColorAlpha(WHITE, 0.5f)
            );
        }
    }

    DrawCircleGradient(position.x, position.y, core_radius * 0.6f,
                       BLACK, BLACK);

    if (m_cooldown_remaining > 0) {
        float cooldown_ratio = m_cooldown_remaining / m_cooldown;
        float arc_angle = 360.0f * (1.0f - cooldown_ratio);

        DrawRing(
            (Vector2){position.x, position.y},
            base_radius * 1.2f,
            base_radius * 1.3f,
            0,
            arc_angle,
            0,
            ColorAlpha(WHITE, 0.6f)
        );
    }

    if (m_is_exit_only) {
        float x_size = base_radius * 0.8f;
        float thickness = 3.0f;

        DrawLineEx(
            (Vector2){position.x - x_size, position.y - x_size},
            (Vector2){position.x + x_size, position.y + x_size},
            thickness,
            WHITE
        );

        DrawLineEx(
            (Vector2){position.x + x_size, position.y - x_size},
            (Vector2){position.x - x_size, position.y + x_size},
            thickness,
            WHITE
        );

        DrawText("EXIT ONLY",
                position.x - 35,
                position.y + base_radius * 1.4f,
                10,
                BLACK);

        float pulse_alpha = 0.5f + 0.5f * sinf(time * 5.0f);

        DrawRing(
            (Vector2){position.x, position.y},
            base_radius * 1.4f,
            base_radius * 1.5f,
            0,
            360,
            0,
            ColorAlpha(BLACK, pulse_alpha * 0.7f)
        );
    } else if (m_linked_teleporter_id != 0) {
        float pulse_size = 0.1f * sinf(time * 3.0f);

        for (int i = 0; i < 8; i++) {
            float angle = i * 45.0f;
            float radius = base_radius * (1.3f + pulse_size);

            float x1 = position.x + cosf(angle * DEG2RAD) * radius;
            float y1 = position.y + sinf(angle * DEG2RAD) * radius;

            float x2 = position.x + cosf(angle * DEG2RAD) * (radius - 8.0f);
            float y2 = position.y + sinf(angle * DEG2RAD) * (radius - 8.0f);

            DrawLineEx(
                (Vector2){x1, y1},
                (Vector2){x2, y2},
                2.0f,
                BLACK
            );
        }

        DrawText("READY",
                position.x - 20,
                position.y + base_radius * 1.4f,
                10,
                BLACK);
    } else {
        float dots_count = (int)(time * 2) % 4;
        char waiting_text[10];

        strcpy(waiting_text, "LINKING");
        for (int i = 0; i < dots_count; i++) {
            strcat(waiting_text, ".");
        }

        DrawText(waiting_text,
                position.x - 30,
                position.y + base_radius * 1.4f,
                10,
                BLACK);
    }
}

void PowerUp::teleport_player(uint64_t player_id) {
    if (m_linked_teleporter_id == 0) {
        auto teleporters = Query::find_where<PowerUp>([this](PowerUp& p) {
            return p.m_type == Type::TELEPORTER &&
                   p.entity_id != this->entity_id;
        });

        if (teleporters.empty()) {
            m_is_exit_only = true;
            return;
        }

        PowerUp& target = teleporters[0].get();
        m_linked_teleporter_id = target.entity_id;
        target.m_linked_teleporter_id = this->entity_id;
    }

    auto target_powerup = Query::try_get<PowerUp>(m_linked_teleporter_id);
    if (!target_powerup) {
        m_linked_teleporter_id = 0;
        return;
    }

    PowerUp& target = target_powerup->get();
    Position& target_pos = Query::get<Position>(m_linked_teleporter_id);
    Position& player_pos = Query::get<Position>(player_id);

    if (auto particle_system = Query::try_find_first<ParticleSystem>()) {
        particle_system->get().spawn_teleport_effect(player_pos, target_pos);
    }

    player_pos.x = target_pos.x;
    player_pos.y = target_pos.y;

    m_cooldown_remaining = m_cooldown;
    target.m_cooldown_remaining = m_cooldown;
}
