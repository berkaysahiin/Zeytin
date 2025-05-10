#include "game/zone.h"

#include "core/raylib_wrapper.h"
#include "game/collider.h"
#include "core/query.h"
#include "game/player_info.h"
#include "game/zone_manager.h"

#include "game/zone_manager.h"

void Zone::on_init() {
    m_in_players.reserve(2);

    const ZoneManager& manager = Query::find_first<ZoneManager>();

    m_max_radius = manager.m_max_radius;
    m_min_radius = manager.m_min_radius;
    m_tick_increase = manager.m_thick_incess_rate;

    log_info() << "m_max_radius: " << m_max_radius << std::endl;
}

void Zone::on_play_update() {
    since_spawn_secs += get_frame_time();

    Query::for_each<PlayerInfo>([this](PlayerInfo& info){
        const auto& player_collider = Query::read<Collider>(info.entity_id);
        const auto& this_collider = Query::read<Collider>(this);
        if(this_collider.intersects(player_collider)) {
            m_in_players.push_back(info.index);
            info.time_spent_zone += get_frame_time();
        }
    });

    if(since_spawn_secs >= vanish_after_secs) {
        Query::remove_entity(entity_id);
    }

    auto& collider = Query::get<Collider>(this);
    if(collider.m_radius < m_max_radius) {
        collider.m_radius += m_tick_increase * get_frame_time();
    }
}

void Zone::on_play_late_update() {
    m_in_players.clear();
}
