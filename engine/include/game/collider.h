#pragma once

#include "variant/variant_base.h"
#include "game/position.h"
#include "game/scale.h"

class Collider : public VariantBase {
    VARIANT(Collider);
    REQUIRES(Scale);

public:
    int m_collider_type = 0; PROPERTY() // 0=None, 1=Rectangle, 2=Circle
    bool m_is_trigger = false; PROPERTY()  
    
    float m_width = 0.0f; PROPERTY()
    float m_height = 0.0f; PROPERTY()
    float m_radius = 0.0f; PROPERTY()

    bool m_static = false; PROPERTY()
    bool m_draw_debug = false; PROPERTY()
    int thickness = 4; PROPERTY();
    
    void on_update() override;
    void on_play_update() override;
    bool intersects(const Collider& other) const;

    Color m_color = BLUE;

    Rectangle get_rectangle() const;
    Vector2 get_circle_center() const;
    inline float get_radius() const { return m_radius; }

    std::function<void(Collider& other)> m_callback;

    inline void set_enable(bool value) { m_enable = value; }
    inline bool is_enable() { return m_enable; }

    float scaled_width() const;
    float scaled_height() const;

    inline bool is_point_in_circle(Vector2 point, Vector2 circle_center, float radius) {
        float distance_squared =
            (point.x - circle_center.x) * (point.x - circle_center.x) +
            (point.y - circle_center.y) * (point.y - circle_center.y);

        return distance_squared <= radius * radius;
    }

private:
    void debug_draw();
    void check_collisions();

    bool m_enable = true;
};

