module;

#include <string>
#include "preparser.h"

export module component.test_rendering;
import zeytin.component;

export class TestRendering : public Component {
public:
    PROPERTY(HELPER="Position (can be moved with WASD/Arrows)")
    float position_x = 400.0f;

    PROPERTY(HELPER="Position (can be moved with WASD/Arrows)")
    float position_y = 300.0f;

    PROPERTY()
    float width = 100.0f;

    PROPERTY()
    float height = 100.0f;

    PROPERTY(HELPER="Movement speed (hold Shift for fast, Ctrl for slow")
    float move_speed = 200.0f;

    PROPERTY(HELPER="Base color (set by number keys, persistent")
    float base_color_r = 0.2f;

    PROPERTY(HELPER="Base color (set by number keys, persistent")
    float base_color_g = 0.6f;

    PROPERTY(HELPER="Base color (set by number keys, persistent")
    float base_color_b = 1.0f;

    PROPERTY()
    float color_a = 1.0f;

    PROPERTY()
    bool is_filled = true;

    PROPERTY()
    float border_thickness = 2.0f;

    PROPERTY()
    std::string label = "Input Test";

    PROPERTY()
    bool visible = true;

    PROPERTY()
    bool follow_mouse = false;  

    void on_update() override;
};
