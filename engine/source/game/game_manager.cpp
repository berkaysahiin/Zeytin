#include "core/query.h"
#include "core/raylib_wrapper.h"

#include "game/game_manager.h"
#include "game/bomb.h"
#include "game/player.h"

int GameManager::s_total_deaths = 0;

void GameManager::on_play_update() {
    m_game_running = is_player_moving();
	draw_death_counter();
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

void GameManager::draw_death_counter() {
    char death_text[32];
    snprintf(death_text, sizeof(death_text), "Deaths: %d", s_total_deaths);
    draw_text(death_text, 10, 10, 50, WHITE);
}
