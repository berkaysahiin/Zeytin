#include "game/zone.h"

#include "game/collider.h"
#include "core/query.h"
#include "game/player_info.h"

void Zone::on_play_start() {
    m_in_players.reserve(2);
}
