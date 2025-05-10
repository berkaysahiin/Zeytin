#include "game/zone_manager.h"

#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "core/zeytin.h"
#include "game/position.h"
#include "game/collider.h"
#include "game/zone.h"

#include "remote_logger/remote_logger.h"

#include "core/raylib_wrapper.h"

::entity_id ZoneManager::spawn_zone(float x, float y, float radius, float vanish_after_secs) {
    auto zone_entity = Query::create_entity();

    Query::add<Position>(zone_entity, x, y);
    
    auto& collider = Query::add<Collider>(zone_entity).value().get();
    collider.m_collider_type = 2; 
    collider.m_radius = radius;
    collider.m_draw_debug = true;
    
    Query::add<Zone>(zone_entity, vanish_after_secs);
    
    
    m_until_next_spawn = m_spawn_every_secs;

    return zone_entity;
}

void ZoneManager::on_play_start() {
}

void ZoneManager::on_play_update() {
    m_until_next_spawn -= get_frame_time();

    if(m_until_next_spawn <= 0) {
        float radius = get_random_value(m_min_radius, m_max_radius);
        float x = get_random_value(radius, VIRTUAL_WIDTH - radius * 3);
        float y = get_random_value(radius, VIRTUAL_HEIGHT - radius * 3);
        float secs = get_random_value(m_min_secs, m_max_secs);
        spawn_zone(x, y, radius, secs);
    }
}
