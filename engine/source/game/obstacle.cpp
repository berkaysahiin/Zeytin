#include "game/obstacle.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/collider.h"

void Obstacle::on_init() {
    fill_color = {100, 100, 100, 255};
    outline_color = {150, 150, 150, 255};
}

void Obstacle::on_update() {
    draw_obstacle();
}

void Obstacle::draw_obstacle() {
    auto& collider = Query::get<Collider>(this);
    Rectangle bounds = collider.get_bounds();
    
    draw_rectangle_rec(bounds, fill_color);
    
    if (show_outline) {
        draw_rectangle_lines_ex(bounds, outline_thickness, outline_color);
    }
}
