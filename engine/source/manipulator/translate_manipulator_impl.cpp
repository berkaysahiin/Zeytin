module;

#include "raylib.h"
#include <cmath>
#include <memory>
#include <string_view>
#include <vector>

module zeytin.manipulator.translate;
import zeytin.manipulator;
import zeytin.game.transform;
import zeytin.raylib;
import zeytin.manipulator.transform_utils;

namespace {
    enum class DragAxis { None, X, Y, Both };
}

struct TranslateManipulator::Impl {
    DragAxis dragging_axis = DragAxis::None;
    float drag_start_x = 0.0f;
    float drag_start_y = 0.0f;
    float drag_start_position_x = 0.0f;
    float drag_start_position_y = 0.0f;

    void handle_interaction(Context& ctx);
    bool is_hovering_axis(const Context& ctx, float mouse_x, float mouse_y, bool is_x_axis) const;
    bool is_hovering_center(const Context& ctx, float mouse_x, float mouse_y) const;
};

TranslateManipulator::TranslateManipulator()
    : m_impl(std::make_unique<Impl>()) {
}

TranslateManipulator::~TranslateManipulator() = default;

void TranslateManipulator::update(Context& ctx) {
#ifdef EDITOR_MODE
    m_impl->handle_interaction(ctx);
#endif

    const float axis_length = 100.0f;
    const float arrow_size = 15.0f;
    const float line_thickness = 4.0f;

    const float pos_x = ctx.transform.position_x;
    const float pos_y = ctx.transform.position_y;

    // x axis, pointing right
    draw_line_ex({pos_x, pos_y}, {pos_x + axis_length, pos_y}, line_thickness, RED);
    draw_triangle({pos_x + axis_length, pos_y},
                  {pos_x + axis_length - arrow_size, pos_y - arrow_size/2},
                  {pos_x + axis_length - arrow_size, pos_y + arrow_size/2}, RED);

    // y axis pointing up
    draw_line_ex({pos_x, pos_y}, {pos_x, pos_y - axis_length}, line_thickness, BLUE);
    draw_triangle({pos_x, pos_y - axis_length},
                  {pos_x - arrow_size/2, pos_y - axis_length + arrow_size},
                  {pos_x + arrow_size/2, pos_y - axis_length + arrow_size}, BLUE);

    // center point
    draw_circle_v({pos_x, pos_y}, 8.0f, YELLOW);
}

bool TranslateManipulator::is_active() const {
    return m_impl->dragging_axis != DragAxis::None;
}

void TranslateManipulator::reset() {
    m_impl->dragging_axis = DragAxis::None;
}

std::string_view TranslateManipulator::get_name() const {
    return "Translate";
}

#ifdef EDITOR_MODE
void TranslateManipulator::Impl::handle_interaction(Context& ctx) {
    Vector2 mouse_pos = get_mouse_position();
    Vector2 world_mouse = get_screen_to_world2d(mouse_pos, ctx.camera);

    if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
        if (is_hovering_center(ctx, world_mouse.x, world_mouse.y)) {
            // draw from center, free movement
            dragging_axis = DragAxis::Both;
            drag_start_x = world_mouse.x;
            drag_start_y = world_mouse.y;
            drag_start_position_x = ctx.transform.position_x;
            drag_start_position_y = ctx.transform.position_y;
        } else if (is_hovering_axis(ctx, world_mouse.x, world_mouse.y, true)) {
            // drag x axis, horizontal only
            dragging_axis = DragAxis::X;
            drag_start_x = world_mouse.x;
            drag_start_y = world_mouse.y;
            drag_start_position_x = ctx.transform.position_x;
            drag_start_position_y = ctx.transform.position_y;
        } else if (is_hovering_axis(ctx, world_mouse.x, world_mouse.y, false)) {
            // draw y axis, vertical only
            dragging_axis = DragAxis::Y;
            drag_start_x = world_mouse.x;
            drag_start_y = world_mouse.y;
            drag_start_position_x = ctx.transform.position_x;
            drag_start_position_y = ctx.transform.position_y;
        }
    }

    // update position locally
    if (dragging_axis != DragAxis::None && is_mouse_button_down(MOUSE_BUTTON_LEFT)) {
        if (dragging_axis == DragAxis::X) {
            float delta_x = world_mouse.x - drag_start_x;
            ctx.transform.position_x = drag_start_position_x + delta_x;
        } else if (dragging_axis == DragAxis::Y) {
            float delta_y = world_mouse.y - drag_start_y;
            ctx.transform.position_y = drag_start_position_y + delta_y;
        } else if (dragging_axis == DragAxis::Both) {
            float delta_x = world_mouse.x - drag_start_x;
            float delta_y = world_mouse.y - drag_start_y;
            ctx.transform.position_x = drag_start_position_x + delta_x;
            ctx.transform.position_y = drag_start_position_y + delta_y;
        }
    }

    // send commands to the editor
    if (dragging_axis != DragAxis::None && is_mouse_button_released(MOUSE_BUTTON_LEFT)) {
        if (dragging_axis == DragAxis::X) {
            transform_utils::send_property_change_command(
                ctx.entity_id,
                "position_x",
                drag_start_position_x,
                ctx.transform.position_x
            );
        } else if (dragging_axis == DragAxis::Y) {
            transform_utils::send_property_change_command(
                ctx.entity_id,
                "position_y",
                drag_start_position_y,
                ctx.transform.position_y
            );
        } else if (dragging_axis == DragAxis::Both) {
            // batch case, center movement
            std::vector<PropertyChange> changes = {
                {"position_x", drag_start_position_x, ctx.transform.position_x},
                {"position_y", drag_start_position_y, ctx.transform.position_y}
            };
            transform_utils::send_batch_property_change_command(ctx.entity_id, changes);
        }
        dragging_axis = DragAxis::None;
    }
}

bool TranslateManipulator::Impl::is_hovering_axis(const Context& ctx, float mouse_x, float mouse_y, bool is_x_axis) const {
    const float axis_length = 100.0f;
    const float hover_threshold = 10.0f;

    const float pos_x = ctx.transform.position_x;
    const float pos_y = ctx.transform.position_y;

    if (is_x_axis) {
        // Check if mouse is near X axis line
        float dist_from_line = std::abs(mouse_y - pos_y);
        return dist_from_line < hover_threshold &&
               mouse_x >= pos_x &&
               mouse_x <= pos_x + axis_length;
    } else {
        // Check if mouse is near Y axis line (pointing up)
        float dist_from_line = std::abs(mouse_x - pos_x);
        return dist_from_line < hover_threshold &&
               mouse_y <= pos_y &&
               mouse_y >= pos_y - axis_length;
    }
}

bool TranslateManipulator::Impl::is_hovering_center(const Context& ctx, float mouse_x, float mouse_y) const {
    const float hover_radius = 12.0f;

    const float pos_x = ctx.transform.position_x;
    const float pos_y = ctx.transform.position_y;

    float dx = mouse_x - pos_x;
    float dy = mouse_y - pos_y;
    float distance = std::sqrt(dx * dx + dy * dy);

    return distance <= hover_radius;
}
#endif
