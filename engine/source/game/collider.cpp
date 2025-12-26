#include "game/collider.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/position.h"
#include "game/scale.h"

void Collider::on_init() {
}

void Collider::on_update() {
    if (show_debug) {
        draw_debug();
    }
}

Rectangle Collider::get_bounds() const {
    auto [position, scale] = Query::read<Position, Scale>(this);
    
    float scaled_width = width * scale.x;
    float scaled_height = height * scale.y;
    
    return {
        position.x - scaled_width / 2,
        position.y - scaled_height / 2,
        scaled_width,
        scaled_height
    };
}

bool Collider::overlaps(const Rectangle& other) const {
    return check_collision_recs(get_bounds(), other);
}

bool Collider::overlaps(const Collider& other) const {
    return check_collision_recs(get_bounds(), other.get_bounds());
}

void Collider::draw_debug() {
    Rectangle bounds = get_bounds();
    draw_rectangle_lines_ex(bounds, 2.0f, {0, 255, 0, 128});
}
