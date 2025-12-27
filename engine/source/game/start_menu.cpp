#include "game/start_menu.h"
#include "core/zeytin.h"

void StartMenu::on_init() {
    m_timer = 0.0f;
    m_game_started = false;
}

void StartMenu::on_update() {
    if (!show_instructions || m_game_started) return;
    
    // Draw overlay
    float alpha = m_timer < fade_in_duration ? m_timer / fade_in_duration : 1.0f;
    
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, 
                 ColorAlpha(background_color, background_opacity * alpha));
    
    draw_instructions();
    
    // Show "press any key" after fade in
    if (m_timer > fade_in_duration) {
        const char* start_text = "Press any key to start";
        int text_width = MeasureText(start_text, instruction_font_size);
        
        float pulse = 0.7f + 0.3f * sinf(get_time() * 3.0f);
        
        draw_text(start_text,
                (VIRTUAL_WIDTH - text_width) / 2,
                VIRTUAL_HEIGHT * 0.75f,
                instruction_font_size,
                ColorAlpha(highlight_color, pulse * alpha));
    }
}

void StartMenu::on_play_update() {
    if (m_game_started) return;
    
    m_timer += get_frame_time();
    
    // Check for any key press to start
    if (m_timer > fade_in_duration && GetKeyPressed() != 0) {
        m_game_started = true;
        show_instructions = false;
    }
}

void StartMenu::draw_instructions() {
    float alpha = m_timer < fade_in_duration ? m_timer / fade_in_duration : 1.0f;
    
    // Title
    int title_width = MeasureText(game_title.c_str(), title_font_size);
    draw_text(game_title.c_str(),
            (VIRTUAL_WIDTH - title_width) / 2,
            VIRTUAL_HEIGHT * 0.2f,
            title_font_size,
            ColorAlpha(title_color, alpha));
    
    // Instructions
    const char* line1 = "1. Collect the DIFFUSER";
    const char* line2 = "2. Reach the BOMB before time runs out";
    const char* line3 = "3. Stand in the circle to DEFUSE";
    
    float y = VIRTUAL_HEIGHT * 0.4f;
    float line_spacing = instruction_font_size * 1.5f;
    
    int w1 = MeasureText(line1, instruction_font_size);
    int w2 = MeasureText(line2, instruction_font_size);
    int w3 = MeasureText(line3, instruction_font_size);
    
    draw_text(line1, (VIRTUAL_WIDTH - w1) / 2, y, 
             instruction_font_size, ColorAlpha(instruction_color, alpha));
    draw_text(line2, (VIRTUAL_WIDTH - w2) / 2, y + line_spacing, 
             instruction_font_size, ColorAlpha(instruction_color, alpha));
    draw_text(line3, (VIRTUAL_WIDTH - w3) / 2, y + line_spacing * 2, 
             instruction_font_size, ColorAlpha(instruction_color, alpha));
    
    // Controls
    const char* controls = "MOVE: A/D or Arrow Keys  |  REWIND: Hold A/Left";
    int ctrl_width = MeasureText(controls, instruction_font_size - 4);
    draw_text(controls,
            (VIRTUAL_WIDTH - ctrl_width) / 2,
            VIRTUAL_HEIGHT * 0.65f,
            instruction_font_size - 4,
            ColorAlpha(highlight_color, alpha * 0.8f));
}
