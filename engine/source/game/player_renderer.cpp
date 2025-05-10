#include "game/player_renderer.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/position.h"
#include "game/player_info.h"
#include "game/scale.h"
#include "game/collider.h"
#include "game/zone.h"
#include "raylib.h"

void PlayerRenderer::on_init() {}

void PlayerRenderer::on_update() {
    draw_player();
    update_position_history();
}

void PlayerRenderer::draw_player() {
    auto [position, info, collider] = Query::read<Position, PlayerInfo, Collider>(this);
    
    float scaled_width = collider.scaled_width();
    float scaled_height = collider.scaled_height();
    
    float time = (float)get_time();
    float pulse = 1.0f + pulse_intensity * sinf(time * pulse_speed);
    
    float visual_width = scaled_width * pulse;
    float visual_height = scaled_height * pulse;
    
    Rectangle rect = {
        position.x - visual_width / 2,
        position.y - visual_height / 2,
        visual_width,
        visual_height
    };
    
    if (show_trail) {
        draw_trail_effect();
    }

    if(info.index == 0) {
        player_color = BLUE;
    }
    else if (info.index == 1) {
        player_color = ORANGE;
    }

    if (corner_radius > 0) {
            float effective_radius = fminf(corner_radius, fminf(visual_width, visual_height) / 2);
            DrawRectangleRounded(rect, effective_radius / visual_width, corner_segments, player_color);
    } else {
        draw_rectangle_rec(rect, player_color);
        if (use_outline && outline_thickness > 0) {
            draw_rectangle_lines_ex(rect, outline_thickness, ColorAlpha(WHITE, 0.7f));
        }
    }

    if (show_player_number) {
        char player_text[8];
        sprintf(player_text, "P%d", info.index + 1);
        
        float text_width = MeasureText(player_text, player_label_size);
        draw_text(player_text, 
                position.x - text_width/2, 
                position.y - player_label_size/2, 
                player_label_size, WHITE);
    }
}

void PlayerRenderer::update_position_history() {
    auto& position = Query::read<Position>(this);
    if (m_position_history.empty() ||
        vector2_distance(m_position_history.front(), {position.x, position.y}) > 3.0f) {

        m_position_history.insert(m_position_history.begin(), {position.x, position.y});

        if (m_position_history.size() > m_max_history_length) {
            m_position_history.pop_back();
        }
    }
}

void PlayerRenderer::draw_trail_effect() {
    if (!show_trail || m_position_history.size() < 2) return;

    for (size_t i = 1; i < m_position_history.size(); i++) {
        float alpha = trail_thickness * pow(m_trail_fade_factor, i);

        float size = trail_thickness * (1.0f - (float)i / m_position_history.size());

        Color trail_point_color = ColorAlpha(player_color, alpha);
        DrawCircleV(m_position_history[i], size, player_color);
    }
}
