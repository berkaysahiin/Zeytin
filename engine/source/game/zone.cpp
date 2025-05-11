#include "game/zone.h"

#include "core/raylib_wrapper.h"
#include "game/collider.h"
#include "core/query.h"
#include "game/player_info.h"
#include "game/zone_manager.h"
#include "game/start_game.h"


void Zone::on_init() {
    const ZoneManager& manager = Query::find_first<ZoneManager>();

    m_max_radius = manager.m_max_radius;
    m_min_radius = manager.m_min_radius;
    m_tick_increase = manager.m_thick_incess_rate;
}

void Zone::on_play_update() {
    const auto& game_started = Query::find_first<StartGame>().game_started;
    if(!game_started) return;

    since_spawn_secs += get_frame_time();

    auto& collider = Query::get<Collider>(this);
    collider.m_color = get_zone_color();

    m_rotation_angle += 15.0f * get_frame_time();
    if (m_rotation_angle > 360.0f) m_rotation_angle -= 360.0f;

    float size_ratio = collider.m_radius / m_max_radius;
    m_polygon_sides = 3 + static_cast<int>(size_ratio * 9.0f);

    draw_polygon_in_zone(collider);

    if(since_spawn_secs >= vanish_after_secs) {
        collider.set_enable(false);
        Query::remove_entity(entity_id);
    }

    if(collider.m_radius < m_max_radius) {
        collider.m_radius += m_tick_increase * get_frame_time();
    }
}

void Zone::on_play_late_update() {}

Color Zone::get_zone_color() const {
    float time_ratio = since_spawn_secs / vanish_after_secs;
    time_ratio = time_ratio < 0.0f ? 0.0f : (time_ratio > 1.0f ? 1.0f : time_ratio);

    Color inner_color, middle_color, outer_color;

    if (time_ratio < 0.4f) {
        inner_color = (Color){0, 255, 255, 230};  
        middle_color = (Color){30, 144, 255, 180}; 
        outer_color = (Color){64, 224, 208, 120};   
    }
    else if (time_ratio < 0.7f) {
        inner_color = (Color){255, 140, 0, 220};    
        middle_color = (Color){255, 191, 0, 160};  
        outer_color = (Color){220, 60, 10, 140};  
    }
    else {
        inner_color = (Color){255, 0, 0, 240};      
        middle_color = (Color){220, 20, 60, 200};  
        outer_color = (Color){148, 0, 211, 160};  
    }

    float base_pulse = sinf(since_spawn_secs * 3.14159f * 2.0f);

    float inner_pulse = 0.9f + 0.1f * sinf(since_spawn_secs * 1.0f * 3.14159f);
    float middle_pulse = 0.8f + 0.2f * sinf(since_spawn_secs * 2.0f * 3.14159f);
    float outer_pulse = 0.7f + 0.3f * sinf(since_spawn_secs * 3.0f * 3.14159f);

    if (time_ratio > 0.7f) {
        float warning_phase = (time_ratio - 0.7f) / 0.3f;
        float warning_pulse = 0.5f + 0.5f * sinf(15.0f * since_spawn_secs * 3.14159f);

        if (time_ratio > 0.9f) {
            warning_pulse = 0.3f + 0.7f * sinf(25.0f * since_spawn_secs * 3.14159f);

            if (fmodf(since_spawn_secs * 10.0f, 1.0f) > 0.5f) {
                inner_color = (Color){255, 255, 255, 200};  
            }
        }

        inner_color.a *= warning_pulse;
        middle_color.a *= warning_pulse;
        outer_color.a *= warning_pulse;
    }

    inner_color.a = (unsigned char)((float)inner_color.a * inner_pulse);
    middle_color.a = (unsigned char)((float)middle_color.a * middle_pulse);
    outer_color.a = (unsigned char)((float)outer_color.a * outer_pulse);

    Color result = inner_color;  

    float middle_influence = 0.3f + 0.2f * time_ratio;
    float outer_influence = 0.1f + 0.3f * time_ratio * time_ratio;

    result.r = (unsigned char)((float)result.r * (1.0f - middle_influence - outer_influence) +
                               (float)middle_color.r * middle_influence +
                               (float)outer_color.r * outer_influence);

    result.g = (unsigned char)((float)result.g * (1.0f - middle_influence - outer_influence) +
                               (float)middle_color.g * middle_influence +
                               (float)outer_color.g * outer_influence);

    result.b = (unsigned char)((float)result.b * (1.0f - middle_influence - outer_influence) +
                               (float)middle_color.b * middle_influence +
                               (float)outer_color.b * outer_influence);

    result.a = (unsigned char)((float)result.a * 0.7f +
                               (float)middle_color.a * 0.2f +
                               (float)outer_color.a * 0.1f);

    result.r = result.r > 255 ? 255 : result.r;
    result.g = result.g > 255 ? 255 : result.g;
    result.b = result.b > 255 ? 255 : result.b;
    result.a = result.a > 255 ? 255 : result.a;

    return result;
}

void Zone::draw_polygon_in_zone(const Collider& collider) {
    const auto& position = Query::read<Position>(this);

    float polygon_radius = collider.m_radius * 0.7f;

    for (int i = 0; i < m_polygon_sides; i++) {
        float angle = m_rotation_angle + i * 360.0f / m_polygon_sides;
        float rad = angle * DEG2RAD;

        float x1 = position.x + cosf(rad) * polygon_radius;
        float y1 = position.y + sinf(rad) * polygon_radius;

        float next_angle = m_rotation_angle + (i + 1) * 360.0f / m_polygon_sides;
        float next_rad = next_angle * DEG2RAD;

        float x2 = position.x + cosf(next_rad) * polygon_radius;
        float y2 = position.y + sinf(next_rad) * polygon_radius;

        Color line_color = collider.m_color;
        line_color.r = fminf(255, line_color.r + 40);
        line_color.g = fminf(255, line_color.g + 40);
        line_color.b = fminf(255, line_color.b + 40);
        line_color.a = 200; 

        DrawLineEx((Vector2){x1, y1}, (Vector2){x2, y2}, 3.0f, line_color);
    }
}
