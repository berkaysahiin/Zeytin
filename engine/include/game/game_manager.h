#pragma once

#include "variant/variant_base.h"

class GameManager : public VariantBase {
    VARIANT(GameManager);

public:
    virtual void on_play_update() override;
    
    bool should_game_run() const { return m_game_running; }

	static void increment_death_count() { s_total_deaths++; }
    static int get_total_deaths() { return s_total_deaths; }
    static void reset_death_count() { s_total_deaths = 0; }

	float pos_x = 10; PROPERTY()
	float pos_y = 10; PROPERTY()
	float size = 50; PROPERTY()

private:
    bool m_game_running = false;
	static int s_total_deaths;
    
    bool is_player_moving();
	void draw_death_counter();  
};
