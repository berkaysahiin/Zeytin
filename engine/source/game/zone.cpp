#include "game/zone.h"

#include "core/raylib_wrapper.h"
#include "game/collider.h"
#include "core/query.h"
#include "game/player_info.h"
#include "game/zone_manager.h"

#include "game/zone_manager.h"

void Zone::on_init() {
    const ZoneManager& manager = Query::find_first<ZoneManager>();

    m_max_radius = manager.m_max_radius;
    m_min_radius = manager.m_min_radius;
    m_tick_increase = manager.m_thick_incess_rate;
}

void Zone::on_play_update() {
    since_spawn_secs += get_frame_time();

    auto& collider = Query::get<Collider>(this);
    collider.m_color = get_zone_color();

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

    float adjusted_ratio = powf(time_ratio, 2.0f); 

    float pulse_speed = 2.0f + 8.0f * adjusted_ratio; 
    float pulse_intensity = 0.2f + 0.3f * adjusted_ratio; 
    float pulse = (1.0f - pulse_intensity) + pulse_intensity * sinf(pulse_speed * since_spawn_secs * 3.14159f);

    Color start_color = {30, 220, 255, 180}; 
    Color end_color = {255, 30, 0, 220};      

    if (time_ratio > 0.8f) {
        float final_intensity = (time_ratio - 0.8f) * 5.0f; 
        end_color.r = 255;
        end_color.g = 30 * (1.0f - final_intensity); 
        end_color.b = 0;

        pulse = (1.0f - 0.5f) + 0.5f * sinf(12.0f * since_spawn_secs * 3.14159f);
    }

    Color result;
    result.r = (start_color.r + (end_color.r - start_color.r) * adjusted_ratio) * pulse;
    result.g = (start_color.g + (end_color.g - start_color.g) * adjusted_ratio) * pulse;
    result.b = (start_color.b + (end_color.b - start_color.b) * adjusted_ratio) * pulse;

    float alpha_pulse = 1.0f;
    if (time_ratio > 0.7f) {
        float warning_phase = (time_ratio - 0.7f) / 0.3f; 
        alpha_pulse = 0.7f + 0.3f * sinf(15.0f * since_spawn_secs * 3.14159f);

        if (time_ratio > 0.9f) {
            alpha_pulse = 0.5f + 0.5f * sinf(25.0f * since_spawn_secs * 3.14159f);
        }
    }

    result.a = (start_color.a + (end_color.a - start_color.a) * adjusted_ratio) * alpha_pulse;

    result.r = result.r > 255 ? 255 : result.r;
    result.g = result.g > 255 ? 255 : result.g;
    result.b = result.b > 255 ? 255 : result.b;
    result.a = result.a > 255 ? 255 : result.a;

    return result;
}
