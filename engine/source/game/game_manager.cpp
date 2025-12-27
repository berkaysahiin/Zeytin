#include "game/game_manager.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/player.h"
#include "game/position.h"

void GameManager::on_play_update() {
    m_game_running = is_player_moving();
}

bool GameManager::is_player_moving() {
    auto player_opt = Query::try_find_first<Player>();
    if (!player_opt) return false;
    
    auto& player = player_opt->get();
    
    // Check if any movement keys are pressed
    bool pressing_left = is_key_down(KEY_A) || is_key_down(KEY_LEFT);
    bool pressing_right = is_key_down(KEY_D) || is_key_down(KEY_RIGHT);
    bool pressing_jump = is_key_down(KEY_SPACE) || is_key_down(KEY_UP);
    
    // Check if player is in the air (not grounded)
    bool in_air = !player.is_grounded();
    
    return pressing_left || pressing_right || pressing_jump || in_air;
}
