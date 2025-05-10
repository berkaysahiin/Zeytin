#include "game/player_renderer.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/position.h"
#include "game/player_info.h"
#include "game/scale.h"
#include "game/collider.h"
#include "game/zone.h"

void PlayerRenderer::on_init() {}

void PlayerRenderer::on_update() {
    draw_player();
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
    
    Color player_color = info.color;

    bool in_zone = info.in_zone;

    if (in_zone && show_zone_effect) {
        float zone_effect = fminf(info.time_spent_zone / zone_effect_duration, 1.0f);
        player_color.r = Lerp(player_color.r, 255, zone_effect * 0.3f);
        player_color.g = Lerp(player_color.g, 215, zone_effect * 0.3f);
        player_color.b = Lerp(player_color.b, 0, zone_effect * 0.3f);
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
    
    if (in_zone && show_zone_effect) {
        draw_zone_effect(Vector2{position.x, position.y}, scaled_width, scaled_height, info.time_spent_zone);
    }
    
    draw_trail_effect(Vector2{position.x, position.y}, scaled_width, scaled_height);
}

void PlayerRenderer::draw_zone_effect(const Vector2& position, float width, float height, float zone_time) {
    float time = (float)get_time();
    float effect_alpha = fminf(zone_time / zone_effect_duration, 0.7f);
    float effect_size = zone_effect_size;
    float wave = sinf(time * zone_effect_speed) * 0.3f + 0.7f;
    
    Color effect_color = {255, 215, 0, (unsigned char)(255 * effect_alpha * wave)};
    
    Rectangle effect_rect = {
        position.x - width * effect_size / 2,
        position.y - height * effect_size / 2,
        width * effect_size,
        height * effect_size
    };
    
    if (corner_radius > 0) {
        float effective_radius = fminf(corner_radius, fminf(width, height) / 2);
        DrawRectangleRounded(effect_rect, effective_radius / (width * effect_size), 
                            corner_segments, effect_color);
    } else {
        draw_rectangle_rec(effect_rect, effect_color);
    }
}

void PlayerRenderer::draw_trail_effect(const Vector2& position, float width, float height) {
    if (!show_trail) return;
    
    float time = (float)get_time();
    
    for (int i = 0; i < trail_count; i++) {
        float line_pos = fmodf((time * trail_speed + i * (1.0f/trail_count)), 1.0f);
        
        float x1 = position.x - width / 2 + line_pos * width * 2;
        float y1 = position.y - height / 2;
        float x2 = x1 - width * trail_length;
        float y2 = position.y + height / 2;
        
        if (x1 < position.x + width / 2 && x2 > position.x - width / 2) {
            float alpha = 0.5f * (1.0f - line_pos);
            DrawLineEx(
                (Vector2){x1, y1}, 
                (Vector2){x2, y2}, 
                trail_thickness, 
                ColorAlpha(trail_color, alpha)
            );
        }
    }
}
