module;

#include "raylib.h"
#include <cmath>
#include <numbers>

module zeytin.game.collider;
import zeytin.raylib;
import zeytin.zeytin;
import zeytin.game.transform;
import zeytin.query;
import zeytin.manipulator.manager;

#ifdef EDITOR_MODE
import zeytin.editor.message;
#endif

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
                const EntityID entity_id = get_id();
                if (entity_id != 0) {
					// editor will sync this info back to the engine
                    send_message_to_editor<EntitySelectedMessage>(entity_id);
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
        .x=pos_x,
        .y=pos_y,
        .width=scaled_width,
        .height=scaled_height
    };

    Vector2 origin = { .x=scaled_width / 2.0F, .y=scaled_height / 2.0F };
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

    const float rotation_rad = -transform.rotation * (std::numbers::pi_v<float> / 180.0F);
    const float cos_r = cosf(rotation_rad);
    const float sin_r = sinf(rotation_rad);

    const float local_x = px - pos_x;
    const float local_y = py - pos_y;

    const float rotated_x = (local_x * cos_r) - (local_y * sin_r);
    const float rotated_y = (local_x * sin_r) + (local_y * cos_r);

    const float half_width = scaled_width / 2.0F;
    const float half_height = scaled_height / 2.0F;

    return rotated_x >= -half_width && rotated_x <= half_width &&
           rotated_y >= -half_height && rotated_y <= half_height;
}
