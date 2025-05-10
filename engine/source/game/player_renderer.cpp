#include "game/player_renderer.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/position.h"
#include "game/player_info.h"
#include "game/scale.h"
#include "game/collider.h"

void PlayerRenderer::on_init() {}

void PlayerRenderer::on_update() {
    draw_player();
}

void PlayerRenderer::draw_player() {
    auto [position, info, collider] = Query::read<Position, PlayerInfo, Collider>(this);
    
    float scaled_width = collider.scaled_width();
    float scaled_height = collider.scaled_height();
    
    float time = (float)get_time();
    float pulse = 1.0f + 0.05f * sinf(time * 5.0f);
    
    scaled_width *= pulse;
    scaled_height *= pulse;
    
    Rectangle rect = {
        position.x - scaled_width / 2,
        position.y - scaled_height / 2,
        scaled_width,
        scaled_height
    };
    
    Color player_color = info.color;
    
    if (corner_radius > 0) {
        DrawRectangleRounded(rect, corner_radius / scaled_width, 10, player_color);
    } else {
        draw_rectangle_rec(rect, player_color);
    }
    
    if (show_player_number) {
        char player_text[8];
        sprintf(player_text, "P%d", info.index + 1);
        
        float text_width = MeasureText(player_text, 20);
        draw_text(player_text, position.x - text_width/2, position.y - 10, 20, WHITE);
    }
    
    draw_player_effects();
}

void PlayerRenderer::draw_player_effects() {
    auto [position, info, collider] = Query::read<Position, PlayerInfo, Collider>(this);
    
    float scaled_width = collider.scaled_width();
    float scaled_height = collider.scaled_height();
    float time = (float)get_time();
    
    if (info.time_spent_zone > 0) {
        float effect_alpha = fminf(info.time_spent_zone / 10.0f, 0.7f);
        float effect_size = 1.3f;
        float wave = sinf(time * 8.0f) * 0.3f + 0.7f;
        
        Color effect_color = {255, 215, 0, (unsigned char)(255 * effect_alpha * wave)};
        Rectangle effect_rect = {
            position.x - scaled_width * effect_size / 2,
            position.y - scaled_height * effect_size / 2,
            scaled_width * effect_size,
            scaled_height * effect_size
        };
        
        if (corner_radius > 0) {
            DrawRectangleRounded(effect_rect, corner_radius / (scaled_width * effect_size), 10, effect_color);
        } else {
            draw_rectangle_rec(effect_rect, effect_color);
        }
    }
    
    for (int i = 0; i < 3; i++) {
        float line_pos = (time * 0.5f + i * 0.33f);
        line_pos = line_pos - floor(line_pos);  
        
        float x1 = position.x - scaled_width / 2 + line_pos * scaled_width * 2;
        float y1 = position.y - scaled_height / 2;
        float x2 = x1 - scaled_width;
        float y2 = position.y + scaled_height / 2;
        
        if (x1 < position.x + scaled_width / 2 && x2 > position.x - scaled_width / 2) {
            DrawLineEx((Vector2){x1, y1}, (Vector2){x2, y2}, 2.0f, ColorAlpha(WHITE, 0.5f));
        }
    }
}
