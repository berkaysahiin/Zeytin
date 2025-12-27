#include "game/time_stopped_indicator.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/end_game.h"
#include "game/game_manager.h"

void TimeStoppedIndicator::on_update() {
    if (!enabled) return;
    
    auto game_mgr_opt = Query::try_find_first<GameManager>();
    if (!game_mgr_opt) return;
    
    // Only draw when game is stopped
    if (game_mgr_opt->get().should_game_run()) {
		m_stoppedDuration = 0;
		return;
    }

	m_stoppedDuration += get_frame_time();

    auto end_game_opt = Query::try_find_first<EndGame>();
    if (!end_game_opt) return;

	if(m_stoppedDuration > activate_threshhold && !end_game_opt->get().is_game_over()) 
	{
    	draw_indicator();
	}
}

void TimeStoppedIndicator::draw_indicator() {
    const char* text = "TIME STOPPED";
    
    // Measure text to center it
    int text_width = MeasureText(text, font_size);
    
    float center_x = VIRTUAL_WIDTH / 2.0f;
    float center_y = VIRTUAL_HEIGHT / 2.0f;
    
    float bg_width = text_width + padding * 2;
    float bg_height = font_size + padding * 2;
    
    draw_rectangle(
        center_x - bg_width / 2,
        center_y - bg_height / 2,
        bg_width,
        bg_height,
        bg_color
    );
    
    // Text
    draw_text(
        text,
        center_x - text_width / 2,
        center_y - font_size / 2,
        font_size,
        text_color
    );
}
