module;

#include "preparser.h"

export module zeytin.game.collider;
import zeytin.component;

export struct CCollider : public Component 
{
    CCollider() = default;

    // Lifecycle methods
    void on_update() override;

    // Draw collider bounds
    void draw_bounds() const;
    
    // Check if mouse is inside collider bounds
    bool is_point_inside(float px, float py) const;

    // Collider size and offset
    PROPERTY(GROUP="Size")
    float width = 100.0f;

    PROPERTY(GROUP="Size")
    float height = 100.0f;

    PROPERTY(GROUP="Offset")
    float offset_x = 0.0f;

    PROPERTY(GROUP="Offset")
    float offset_y = 0.0f;

    PROPERTY(GROUP="Debug")
    bool show_bounds = true;
};
