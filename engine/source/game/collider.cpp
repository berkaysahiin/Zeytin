#include "game/collider.h"
#include "core/raylib_wrapper.h"
#include "game/position.h"

#include "core/query.h"
#include "raylib.h"
#include "raymath.h"

enum class ColliderType : int {
    None = 0,
    Rectangle = 1,
    Circle = 2,
};

void Collider::on_update() {
    debug_draw();
}

void Collider::on_play_update() {
    check_collisions();
}

void Collider::check_collisions() {
    auto& position = Query::read<Position>(this);

    Query::for_each<Collider>([this](Collider& other) {
        if(!m_enable || !other.m_enable) {
            return;
        }

        if (other.entity_id == entity_id) {
            return;
        }

        if (other.m_collider_type == (int)ColliderType::None) {
            return;
        }

        if (this->intersects(other)) {
            if(m_callback) {
                m_callback(other);
            }
        }
    });
}

bool Collider::intersects(const Collider& other) const {
    if (m_collider_type == 0 || other.m_collider_type == 0) {
        return false;
    }

    if (m_collider_type == 1 && other.m_collider_type == 1) {
        const auto& this_rect = get_rectangle();
        const auto& other_rect = other.get_rectangle();
        return CheckCollisionRecs(this_rect, other_rect);
    }

    if (m_collider_type == 2 && other.m_collider_type == 2) {
        Vector2 center1 = get_circle_center();
        Vector2 center2 = other.get_circle_center();
        float distance = Vector2Distance(center1, center2);
        return distance <= (m_radius + other.m_radius);
    }

    if ((m_collider_type == 1 && other.m_collider_type == 2) ||
        (m_collider_type == 2 && other.m_collider_type == 1)) {

        const Collider& rect_collider = (m_collider_type == 1) ? *this : other;
        const Collider& circle_collider = (m_collider_type == 2) ? *this : other;

        Rectangle rect = rect_collider.get_rectangle();
        Vector2 center = circle_collider.get_circle_center();
        float radius = circle_collider.m_radius;

        float closest_x = fmaxf(rect.x, fminf(center.x, rect.x + rect.width));
        float closest_y = fmaxf(rect.y, fminf(center.y, rect.y + rect.height));

        float distance = Vector2Distance(center, {closest_x, closest_y});

        return distance <= radius;
    }

    return false;
}

Rectangle Collider::get_rectangle() const {
    const auto& position = Query::get<Position>(this);

    return Rectangle{
        position.x - m_width / 2, 
        position.y - m_height / 2,
        m_width, 
        m_height
    };
}

Vector2 Collider::get_circle_center() const {
    const auto& position = Query::get<Position>(this);

    return Vector2{
        position.x,
        position.y
    };
}

void Collider::debug_draw() {
    if (!m_draw_debug) {
        return;
    }

    const auto& position = Query::get<Position>(this);
    Color color = m_is_trigger ? YELLOW : BLUE;

    switch (m_collider_type) {
        case (int)ColliderType::Rectangle:
            draw_rectangle_rec(get_rectangle(), color);
            break;
        case (int)ColliderType::Circle:
            DrawRing(
                (Vector2){ position.x, position.y },  
                m_radius - thickness / 2.0f,         
                m_radius + thickness / 2.0f,        
                0,                                 
                360,                              
                360,                             
                color                           
            );

            break;
        default:
            break;
    }
}
