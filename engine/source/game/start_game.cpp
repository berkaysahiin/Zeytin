#include "game/start_game.h"
#include "core/query.h"
#include "core/zeytin.h"
#include "game/end_game.h"

void StartGame::on_init() {
    m_timer = 0.0f;
    game_started = false;
}

void StartGame::on_play_update() {
    m_timer += get_frame_time();

    if (show_instructions) {
        float alpha = 1.0f;
        
        if (m_timer < fade_in_duration) {
            alpha = m_timer / fade_in_duration;
        }
        
        DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, 
                     ColorAlpha(background_color, background_opacity * alpha));
        
        draw_instructions();
        
        if (show_controls) {
            draw_controls();
        }
        
        if (m_timer > fade_in_duration) {
            const char* start_text = "Press any key to start";
            int text_width = MeasureText(start_text, instruction_font_size);
            
            float y_pos = VIRTUAL_HEIGHT * 0.85f;
            float pulse = 0.7f + 0.3f * sinf(get_time() * 3.0f);
            
            draw_text(start_text,
                    (VIRTUAL_WIDTH - text_width) / 2,
                    y_pos,
                    instruction_font_size,
                    ColorAlpha(highlight_color, pulse * alpha));
            
            if (GetKeyPressed() != 0) {
                game_started = true;
                show_instructions = false;
            }
        }
    }
}

void StartGame::draw_instructions() {
    float alpha = m_timer < fade_in_duration ? m_timer / fade_in_duration : 1.0f;
    
    int title_width = MeasureText(game_title.c_str(), title_font_size);
    draw_text(game_title.c_str(),
            (VIRTUAL_WIDTH - title_width) / 2,
            VIRTUAL_HEIGHT * 0.2f,
            title_font_size,
            ColorAlpha(title_color, alpha));
    
    int instr_width = MeasureText(instruction_text.c_str(), instruction_font_size);
    draw_text(instruction_text.c_str(),
            (VIRTUAL_WIDTH - instr_width) / 2,
            VIRTUAL_HEIGHT * 0.35f,
            instruction_font_size,
            ColorAlpha(instruction_color, alpha));
    
    const char* point_text = "Collect powerups to gain advantages!";
    int point_width = MeasureText(point_text, instruction_font_size);
    draw_text(point_text,
            (VIRTUAL_WIDTH - point_width) / 2,
            VIRTUAL_HEIGHT * 0.45f,
            instruction_font_size,
            ColorAlpha(instruction_color, alpha));
    
    const char* warning_text = "Stay out of zones too long and you'll lose points!";
    int warning_width = MeasureText(warning_text, instruction_font_size);
    draw_text(warning_text,
            (VIRTUAL_WIDTH - warning_width) / 2,
            VIRTUAL_HEIGHT * 0.55f,
            instruction_font_size,
            ColorAlpha(highlight_color, alpha));
}

void StartGame::draw_controls() {
    float alpha = m_timer < fade_in_duration ? m_timer / fade_in_duration : 1.0f;
    float y_pos = VIRTUAL_HEIGHT * 0.7f;
    float line_height = controls_font_size * 1.3f;
    
    const char* p1_controls = "Player 1: Arrow Keys";
    const char* p2_controls = "Player 2: WASD Keys";
    
    int p1_width = MeasureText(p1_controls, controls_font_size);
    int p2_width = MeasureText(p2_controls, controls_font_size);
    
    draw_text(p1_controls,
            (VIRTUAL_WIDTH - p1_width) / 2,
            y_pos,
            controls_font_size,
            ColorAlpha(instruction_color, alpha));
    
    draw_text(p2_controls,
            (VIRTUAL_WIDTH - p2_width) / 2,
            y_pos + line_height,
            controls_font_size,
            ColorAlpha(instruction_color, alpha));
}
