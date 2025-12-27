#include "game/rewind_effect.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/time_controller.h"

void RewindEffect::on_init() {
    m_timer = 0.0f;
}

void RewindEffect::on_update() {
    if (!enabled) return;
    
    auto time_controller_opt = Query::try_find_first<TimeController>();
    if (!time_controller_opt) return;
    
    if (!time_controller_opt->get().is_rewinding()) return;
    
    m_timer += get_frame_time();
    
    draw_rewind_overlay();
    draw_center_panel();
}

void RewindEffect::draw_rewind_overlay() {
    //draw_rectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, rewind_tint);
}

void RewindEffect::draw_center_panel() {
    float center_x = VIRTUAL_WIDTH / 2.0f;
    float center_y = VIRTUAL_HEIGHT / 2.0f;
    
    float total_width = arrow_count * arrow_size + (arrow_count - 1) * arrow_spacing + 60.0f;
    float panel_height = arrow_size + 40.0f;
    
    draw_rectangle(
        center_x - total_width / 2,
        center_y - panel_height / 2,
        total_width,
        panel_height,
        arrow_bg_color
    );
    
    float pulse = 1.0f + sin(m_timer * pulse_speed) * pulse_intensity;
    
    float start_x = center_x - ((arrow_count - 1) * (arrow_size + arrow_spacing)) / 2.0f;
    
    for (int i = 0; i < arrow_count; i++) {
        float x = start_x + i * (arrow_size + arrow_spacing);
        draw_arrow(x, center_y, pulse);
    }
    
    const char* text = "REWINDING";
    int font_size = 24;
    int text_width = MeasureText(text, font_size);
    
    draw_text(
        text,
        center_x - text_width / 2,
        center_y + panel_height / 2 + 10,
        font_size,
        arrow_color
    );
}

void RewindEffect::draw_arrow(float x, float y, float scale) {
    float half_size = (arrow_size / 2.0f) * scale;
    
    Vector2 tip = {x - half_size, y};
    Vector2 top = {x + half_size, y - half_size};
    Vector2 bottom = {x + half_size, y + half_size};
    
    DrawTriangle(tip, bottom, top, arrow_color);
}
