module;

#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <memory>
#include <string_view>
#include <vector>

module zeytin.manipulator.scale;
import zeytin.manipulator;
import zeytin.game.transform;
import zeytin.raylib;
import zeytin.manipulator.transform_utils;

namespace {
    constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;
    enum class DragAxis { None, X, Y, Both };
}

struct ScaleManipulator::Impl {
    DragAxis dragging_axis = DragAxis::None;
    float drag_start_x = 0.0f;
    float drag_start_y = 0.0f;
    float start_scale_x = 1.0f;
    float start_scale_y = 1.0f;
    bool skip_drag_update = false;

    void handle_interaction(Context& ctx);
    bool is_hovering_handle(const Context& ctx, float mouse_x, float mouse_y, DragAxis axis) const;
};

ScaleManipulator::ScaleManipulator()
    : m_impl(std::make_unique<Impl>()) {
}

ScaleManipulator::~ScaleManipulator() = default;

void ScaleManipulator::update(Context& ctx) {
#ifdef EDITOR_MODE
    m_impl->handle_interaction(ctx);
#endif

    const float axis_length = 80.0f;
    const float handle_size = 12.0f;
    const float line_thickness = 3.0f;

    const float pos_x = ctx.transform.position_x;
    const float pos_y = ctx.transform.position_y;

    const float rotation_rad = ctx.transform.rotation * kDegToRad;
    const Vector2 axis_x = {std::cos(rotation_rad), std::sin(rotation_rad)};
    const Vector2 axis_y = {std::sin(rotation_rad), -std::cos(rotation_rad)};

    const Vector2 x_end = {pos_x + axis_x.x * axis_length, pos_y + axis_x.y * axis_length};
    const Vector2 y_end = {pos_x + axis_y.x * axis_length, pos_y + axis_y.y * axis_length};

    Vector2 mouse_pos = get_mouse_position();
    Vector2 world_mouse = get_screen_to_world2d(mouse_pos, ctx.camera);

    const bool hover_center = m_impl->is_hovering_handle(ctx, world_mouse.x, world_mouse.y, DragAxis::Both);
    const bool hover_x = m_impl->is_hovering_handle(ctx, world_mouse.x, world_mouse.y, DragAxis::X);
    const bool hover_y = m_impl->is_hovering_handle(ctx, world_mouse.x, world_mouse.y, DragAxis::Y);

    Color center_color = (m_impl->dragging_axis == DragAxis::Both) ? GREEN : (hover_center ? YELLOW : LIGHTGRAY);
    Color x_color = (m_impl->dragging_axis == DragAxis::X) ? GREEN : (hover_x ? YELLOW : RED);
    Color y_color = (m_impl->dragging_axis == DragAxis::Y) ? GREEN : (hover_y ? YELLOW : BLUE);

    draw_line_ex({pos_x, pos_y}, x_end, line_thickness, x_color);
    draw_line_ex({pos_x, pos_y}, y_end, line_thickness, y_color);

    Rectangle x_handle = {x_end.x - handle_size / 2.0f, x_end.y - handle_size / 2.0f, handle_size, handle_size};
    Rectangle y_handle = {y_end.x - handle_size / 2.0f, y_end.y - handle_size / 2.0f, handle_size, handle_size};
    Rectangle center_handle = {pos_x - handle_size / 2.0f, pos_y - handle_size / 2.0f, handle_size, handle_size};

    draw_rectangle_rec(x_handle, x_color);
    draw_rectangle_rec(y_handle, y_color);
    draw_rectangle_rec(center_handle, center_color);
}

bool ScaleManipulator::is_active() const {
    return m_impl->dragging_axis != DragAxis::None;
}

void ScaleManipulator::reset() {
    m_impl->dragging_axis = DragAxis::None;
}

std::string_view ScaleManipulator::get_name() const {
    return "Scale";
}

#ifdef EDITOR_MODE
void ScaleManipulator::Impl::handle_interaction(Context& ctx) {
    Vector2 mouse_pos = get_mouse_position();
    Vector2 world_mouse = get_screen_to_world2d(mouse_pos, ctx.camera);

    if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
        if (is_hovering_handle(ctx, world_mouse.x, world_mouse.y, DragAxis::Both)) {
            dragging_axis = DragAxis::Both;
        } else if (is_hovering_handle(ctx, world_mouse.x, world_mouse.y, DragAxis::X)) {
            dragging_axis = DragAxis::X;
        } else if (is_hovering_handle(ctx, world_mouse.x, world_mouse.y, DragAxis::Y)) {
            dragging_axis = DragAxis::Y;
        }

        if (dragging_axis != DragAxis::None) {
            drag_start_x = world_mouse.x;
            drag_start_y = world_mouse.y;
            start_scale_x = ctx.transform.scale_x;
            start_scale_y = ctx.transform.scale_y;
            skip_drag_update = true;
        }
    }

    if (dragging_axis != DragAxis::None && is_mouse_button_down(MOUSE_BUTTON_LEFT)) {
        const float axis_length = 80.0f;
        const float rotation_rad = ctx.transform.rotation * kDegToRad;
        const Vector2 axis_x = {std::cos(rotation_rad), std::sin(rotation_rad)};
        const Vector2 axis_y = {std::sin(rotation_rad), -std::cos(rotation_rad)};

        const float delta_world_x = world_mouse.x - drag_start_x;
        const float delta_world_y = world_mouse.y - drag_start_y;

        if (skip_drag_update) {
            const float movement = std::abs(delta_world_x) + std::abs(delta_world_y);
            if (movement < 0.0001f) {
                return;
            }
            skip_drag_update = false;
        }

        const float delta_x = delta_world_x * axis_x.x + delta_world_y * axis_x.y;
        const float delta_y = delta_world_x * axis_y.x + delta_world_y * axis_y.y;

        if (dragging_axis == DragAxis::X) {
            ctx.transform.scale_x = std::max(0.01f, start_scale_x + delta_x / axis_length);
        } else if (dragging_axis == DragAxis::Y) {
            ctx.transform.scale_y = std::max(0.01f, start_scale_y + delta_y / axis_length);
        } else if (dragging_axis == DragAxis::Both) {
            const float delta = (delta_x + delta_y) * 0.5f;
            const float scale = std::max(0.01f, start_scale_x + delta / axis_length);
            ctx.transform.scale_x = scale;
            ctx.transform.scale_y = scale;
        }
    }

    if (dragging_axis != DragAxis::None && is_mouse_button_released(MOUSE_BUTTON_LEFT)) {
        if (dragging_axis == DragAxis::X) {
            transform_utils::send_property_change_command(
                ctx.entity_id,
                "scale_x",
                start_scale_x,
                ctx.transform.scale_x
            );
        } else if (dragging_axis == DragAxis::Y) {
            transform_utils::send_property_change_command(
                ctx.entity_id,
                "scale_y",
                start_scale_y,
                ctx.transform.scale_y
            );
        } else if (dragging_axis == DragAxis::Both) {
            std::vector<PropertyChange> changes = {
                {"scale_x", start_scale_x, ctx.transform.scale_x},
                {"scale_y", start_scale_y, ctx.transform.scale_y}
            };
            transform_utils::send_batch_property_change_command(ctx.entity_id, changes);
        }
        dragging_axis = DragAxis::None;
        skip_drag_update = false;
    }
}

bool ScaleManipulator::Impl::is_hovering_handle(const Context& ctx, float mouse_x, float mouse_y, DragAxis axis) const {
    const float axis_length = 80.0f;
    const float handle_size = 12.0f;
    const float half = handle_size / 2.0f;

    const float pos_x = ctx.transform.position_x;
    const float pos_y = ctx.transform.position_y;

    const float rotation_rad = ctx.transform.rotation * kDegToRad;
    const Vector2 axis_x = {std::cos(rotation_rad), std::sin(rotation_rad)};
    const Vector2 axis_y = {std::sin(rotation_rad), -std::cos(rotation_rad)};

    float handle_x = pos_x;
    float handle_y = pos_y;

    if (axis == DragAxis::X) {
        handle_x = pos_x + axis_x.x * axis_length;
        handle_y = pos_y + axis_x.y * axis_length;
    } else if (axis == DragAxis::Y) {
        handle_x = pos_x + axis_y.x * axis_length;
        handle_y = pos_y + axis_y.y * axis_length;
    }

    return std::abs(mouse_x - handle_x) <= half && std::abs(mouse_y - handle_y) <= half;
}
#endif
