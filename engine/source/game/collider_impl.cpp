module;

#include "raylib.h"
#include <cmath>

module zeytin.game.collider;
import zeytin.raylib;
import zeytin.zeytin;
import zeytin.game.transform;
import zeytin.query;
import zeytin.manipulator.manager;
import zeytin.editor.message;
import zeytin.entity;

void CCollider::on_update() {
    if (show_bounds) {
        draw_bounds();
    }

#ifdef EDITOR_MODE
    if (!Zeytin::get().is_play_mode()) {
        if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
            const Vector2 mouse_pos = get_mouse_position();
            const Vector2 world_mouse = get_screen_to_world2d(mouse_pos, Zeytin::get().get_camera());

            if (is_point_inside(world_mouse.x, world_mouse.y)) {
                const EntityID id = get_id();
                if (id != 0) {
					// editor will sync this info back to the engine
                    send_to_editor(EntitySelectedMessage{id});
                }
            }
        }
    }
#endif
}

void CCollider::draw_bounds() const {
    const auto transform_opt = Query::try_get<CTransform>(this);

    if (!transform_opt) {
        return; 
    }

    const CTransform& transform = transform_opt->get();

    const float scaled_width = std::abs(width * transform.scale_x);
    const float scaled_height = std::abs(height * transform.scale_y);
    const float scaled_offset_x = offset_x * transform.scale_x;
    const float scaled_offset_y = offset_y * transform.scale_y;

    const float pos_x = transform.position_x + scaled_offset_x;
    const float pos_y = transform.position_y + scaled_offset_y;

    const Rectangle rec = {
        pos_x,
        pos_y,
        scaled_width,
        scaled_height
    };

    Vector2 origin = { scaled_width / 2.0f, scaled_height / 2.0f }; // Center the rectangle

#ifdef EDITOR_MODE
    // In editor mode, draw with green in edit mode, yellow in play mode
    if (!Zeytin::get().is_play_mode()) {
        DrawRectanglePro(rec, origin, transform.rotation, GREEN);
        // Draw center point
        draw_circle_v({pos_x, pos_y}, 3.0f, GREEN);
    } else {
        DrawRectanglePro(rec, origin, transform.rotation, YELLOW);
    }
#else
    // In standalone mode, draw with yellow
    DrawRectanglePro(rec, origin, transform.rotation, YELLOW);
#endif
}

bool CCollider::is_point_inside(float px, float py) const {
    const auto transform_opt = Query::try_get<CTransform>(this);

    if (!transform_opt) {
        return false; 
    }

    const CTransform& transform = transform_opt->get();

    const float scaled_width = std::abs(width * transform.scale_x);
    const float scaled_height = std::abs(height * transform.scale_y);
    const float scaled_offset_x = offset_x * transform.scale_x;
    const float scaled_offset_y = offset_y * transform.scale_y;

    const float pos_x = transform.position_x + scaled_offset_x;
    const float pos_y = transform.position_y + scaled_offset_y;

    const float rotation_rad = -transform.rotation * (3.14159265358979323846f / 180.0f);
    const float cos_r = cosf(rotation_rad);
    const float sin_r = sinf(rotation_rad);

    const float local_x = px - pos_x;
    const float local_y = py - pos_y;

    const float rotated_x = local_x * cos_r - local_y * sin_r;
    const float rotated_y = local_x * sin_r + local_y * cos_r;

    const float half_width = scaled_width / 2.0f;
    const float half_height = scaled_height / 2.0f;

    return rotated_x >= -half_width && rotated_x <= half_width &&
           rotated_y >= -half_height && rotated_y <= half_height;
}
