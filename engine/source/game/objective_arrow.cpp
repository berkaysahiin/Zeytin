#include "game/objective_arrow.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/player.h"
#include "game/diffuser.h"
#include "game/bomb.h"
#include "game/position.h"
#include <cmath>

void ObjectiveArrow::on_init() {
    m_bounce_timer = 0.0f;
}

void ObjectiveArrow::on_update() {
    m_bounce_timer += get_frame_time();
    draw_arrow();
}

void ObjectiveArrow::draw_arrow() {
    auto player_opt = Query::try_find_first<Player>();
    if (!player_opt) return;
    
    bool has_diffuser = player_opt->get().has_diffuser();
    
    if (!has_diffuser) {
        auto diffuser_opt = Query::try_find_first<Diffuser>();
        if (diffuser_opt) {
            auto& diffuser_pos = Query::get<Position>(&diffuser_opt->get());
            draw_arrow_at(diffuser_pos.x, diffuser_pos.y);
        }
    } else {
        auto bomb_opt = Query::try_find_first<Bomb>();
        if (bomb_opt && !bomb_opt->get().is_defused()) {
            auto& bomb_pos = Query::get<Position>(&bomb_opt->get());
            draw_arrow_at(bomb_pos.x, bomb_pos.y);
        }
    }
}

void ObjectiveArrow::draw_arrow_at(float x, float y) {
    float bounce = sin(m_bounce_timer * bounce_speed) * bounce_amount;
    float arrow_y = y - arrow_height + bounce;
    
    Vector2 tip = {x, arrow_y + arrow_size};
    Vector2 left = {x - arrow_size / 2, arrow_y};
    Vector2 right = {x + arrow_size / 2, arrow_y};
    
    DrawTriangle(tip, left, right, arrow_color);
    
    Color outline = arrow_color;
    outline.a = 255;
    draw_line(tip.x, tip.y, left.x, left.y, outline);
    draw_line(tip.x, tip.y, right.x, right.y, outline);
    draw_line(left.x, left.y, right.x, right.y, outline);
}
