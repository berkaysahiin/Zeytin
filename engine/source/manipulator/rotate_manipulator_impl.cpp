module;

#include "raylib.h"
#include <cmath>
#include <memory>
#include <string_view>

module zeytin.manipulator.rotate;
import zeytin.manipulator;
import zeytin.game.transform;
import zeytin.raylib;
import zeytin.manipulator.transform_utils;

struct RotateManipulator::Impl {
    bool is_rotating = false;
    float start_angle = 0.0f;
    float start_rotation = 0.0f;

    void handle_interaction(Context& ctx);
    bool is_hovering_circle(const Context& ctx, float mouse_x, float mouse_y) const;
    float get_angle_to_mouse(const Context& ctx, float mouse_x, float mouse_y) const;
};

RotateManipulator::RotateManipulator()
    : m_impl(std::make_unique<Impl>()) {
}

RotateManipulator::~RotateManipulator() = default;

void RotateManipulator::update(Context& ctx) {
#ifdef EDITOR_MODE
    m_impl->handle_interaction(ctx);
#endif

    // draw rotation gizmo
    const float circle_radius = 80.0f;

    const float pos_x = ctx.transform.position_x;
    const float pos_y = ctx.transform.position_y;

    Vector2 mouse_pos = get_mouse_position();
    Vector2 world_mouse = get_screen_to_world2d(mouse_pos, ctx.camera);
    bool is_hovering = m_impl->is_hovering_circle(ctx, world_mouse.x, world_mouse.y);

    // draw circle with color based on hover/active state
    Color circle_color = m_impl->is_rotating ? GREEN : (is_hovering ? YELLOW : SKYBLUE);
    draw_circle_lines(pos_x, pos_y, circle_radius, circle_color);

    // draw a slightly thicker ring for better visibility
    draw_circle_lines(pos_x, pos_y, circle_radius + 1.0f, circle_color);

    // draw rotation indicator line from center
    float rotation_rad = ctx.transform.rotation * DEG2RAD;
    float indicator_x = pos_x + std::cos(rotation_rad) * circle_radius;
    float indicator_y = pos_y + std::sin(rotation_rad) * circle_radius;
    draw_line_ex({pos_x, pos_y}, {indicator_x, indicator_y}, 2.0f, circle_color);

    // draw small circle at the end of indicator
    draw_circle_v({indicator_x, indicator_y}, 5.0f, circle_color);
}

bool RotateManipulator::is_active() const {
    return m_impl->is_rotating;
}

void RotateManipulator::reset() {
    m_impl->is_rotating = false;
}

std::string_view RotateManipulator::get_name() const {
    return "Rotate";
}

#ifdef EDITOR_MODE
void RotateManipulator::Impl::handle_interaction(Context& ctx) {
    Vector2 mouse_pos = get_mouse_position();
    Vector2 world_mouse = get_screen_to_world2d(mouse_pos, ctx.camera);

    // start rotating
    if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
        if (is_hovering_circle(ctx, world_mouse.x, world_mouse.y)) {
            is_rotating = true;
            start_angle = get_angle_to_mouse(ctx, world_mouse.x, world_mouse.y);
            start_rotation = ctx.transform.rotation;
        }
    }

    // during rotation
    if (is_rotating && is_mouse_button_down(MOUSE_BUTTON_LEFT)) {
        float current_angle = get_angle_to_mouse(ctx, world_mouse.x, world_mouse.y);
        float angle_delta = current_angle - start_angle;

        // Update rotation
        ctx.transform.rotation = start_rotation + angle_delta;

        // Normalize to 0-360 range for cleaner values
        while (ctx.transform.rotation < 0.0f) {
            ctx.transform.rotation += 360.0f;
        }
        while (ctx.transform.rotation >= 360.0f) {
            ctx.transform.rotation -= 360.0f;
        }
    }

    // end rotation
    if (is_rotating && is_mouse_button_released(MOUSE_BUTTON_LEFT)) {
        transform_utils::send_property_change_command(
            ctx.entity_id,
            "rotation",
            start_rotation,
            ctx.transform.rotation
        );
        is_rotating = false;
    }
}

float RotateManipulator::Impl::get_angle_to_mouse(const Context& ctx, float mouse_x, float mouse_y) const {
    const float pos_x = ctx.transform.position_x;
    const float pos_y = ctx.transform.position_y;

    float dx = mouse_x - pos_x;
    float dy = mouse_y - pos_y;

    // atan2 returns radians, convert to degrees
    float angle_rad = std::atan2(dy, dx);
    return angle_rad * RAD2DEG;
}
#endif

bool RotateManipulator::Impl::is_hovering_circle(const Context& ctx, float mouse_x, float mouse_y) const {
    const float circle_radius = 80.0f;
    const float hover_threshold = 10.0f;

    const float pos_x = ctx.transform.position_x;
    const float pos_y = ctx.transform.position_y;

    float dx = mouse_x - pos_x;
    float dy = mouse_y - pos_y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // check if mouse is near the circle edge
    return std::abs(distance - circle_radius) < hover_threshold;
}
