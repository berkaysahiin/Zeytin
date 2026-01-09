module;

#include "raylib.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <cmath>

module zeytin.game.collider;
import zeytin.raylib;
import zeytin.zeytin;
import zeytin.game.transform;
import zeytin.query;

#ifdef EDITOR_MODE
import zeytin.editor.event;
#endif

void CCollider::on_update() {
    if (show_bounds) {
        draw_bounds();
    }

#ifdef EDITOR_MODE
    // In editor mode (not play mode), check for click to select entity
    if (!Zeytin::get().is_play_mode()) {
        if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse_pos = get_mouse_position();
            Vector2 world_mouse = get_screen_to_world2d(mouse_pos, Zeytin::get().get_camera());

            if (is_point_inside(world_mouse.x, world_mouse.y)) {
                // Send entity selected event to editor
                rapidjson::Document msg;
                msg.SetObject();
                auto& alloc = msg.GetAllocator();

                msg.AddMember("type", "entity_selected_from_engine", alloc);
                msg.AddMember("entity_id", get_id(), alloc);

                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                msg.Accept(writer);

                EditorEventBus::get().publish<std::string>(EditorEvent::SyncEditor, buffer.GetString());
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

    const float pos_x = transform.position_x + offset_x;
    const float pos_y = transform.position_y + offset_y;

    const Rectangle rec = {
        pos_x,           
        pos_y,          
        width,     
        height           
    };

    Vector2 origin = { width / 2.0f, height / 2.0f }; // Center the rectangle

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

    const float pos_x = transform.position_x + offset_x;
    const float pos_y = transform.position_y + offset_y;

    const float rotation_rad = -transform.rotation * (3.14159265358979323846f / 180.0f);
    const float cos_r = cosf(rotation_rad);
    const float sin_r = sinf(rotation_rad);

    const float local_x = px - pos_x;
    const float local_y = py - pos_y;

    const float rotated_x = local_x * cos_r - local_y * sin_r;
    const float rotated_y = local_x * sin_r + local_y * cos_r;

    const float half_width = width / 2.0f;
    const float half_height = height / 2.0f;

    return rotated_x >= -half_width && rotated_x <= half_width &&
           rotated_y >= -half_height && rotated_y <= half_height;
}
