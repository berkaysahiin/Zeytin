#include "game/game_manager.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/bomb.h"
#include "game/player.h"

void GameManager::on_play_update() {
    m_game_running = is_player_moving();
}

bool GameManager::is_player_moving() {
    auto bomb_opt = Query::try_find_first<Bomb>();
	if (bomb_opt) {
		const bool is_defusing = bomb_opt->get().is_being_defused();
		if (is_defusing) return true;
	}

    auto player_opt = Query::try_find_first<Player>();
    if (!player_opt) return false;

	const Vector2 velocity = player_opt->get().get_velocity();
	const float length = vector2_length(velocity);

	return length > 0.01f;
}
