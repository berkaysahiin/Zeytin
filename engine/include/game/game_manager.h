#pragma once

#include "variant/variant_base.h"

class GameManager : public VariantBase {
    VARIANT(GameManager);

public:
    virtual void on_play_update() override;
    
    bool should_game_run() const { return m_game_running; }

private:
    bool m_game_running = false;
    
    bool is_player_moving();
};
