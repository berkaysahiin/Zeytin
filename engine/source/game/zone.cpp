#include "game/zone.h"

#include "game/collider.h"
#include "core/query.h"
#include "game/player_info.h"

void Zone::on_play_start() {
    m_in_players.reserve(2);

}

void Zone::on_play_update() {
    Query::for_each<PlayerInfo>([this](PlayerInfo& info){
        const auto& player_collider = Query::read<Collider>(info.entity_id);
        const auto& this_collider = Query::read<Collider>(this);
        if(this_collider.intersects(player_collider)) {
            m_in_players.push_back(info.index);
        }
    });
}

void Zone::on_play_late_update() {
    m_in_players.clear();
}
