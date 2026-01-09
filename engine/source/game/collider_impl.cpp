module;

#include "raylib.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

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
    // Query CTransform component to get position
    auto transform_opt = Query::try_get<CTransform>(this);
    
    if (!transform_opt) {
        return; // No transform, can't draw
    }
    
    CTransform& transform = transform_opt->get();
    
    // Get position from transform and apply offset
    float pos_x = transform.position_x + offset_x;
    float pos_y = transform.position_y + offset_y;
    
    // Draw collider as a rectangle centered at position
    float left = pos_x - width / 2.0f;
    float top = pos_y - height / 2.0f;
    
#ifdef EDITOR_MODE
    // In editor mode, draw with green in edit mode, yellow in play mode
    if (!Zeytin::get().is_play_mode()) {
        draw_rectangle_lines_ex({left, top, width, height}, 2.0f, GREEN);
        // Draw center point
        draw_circle_v({pos_x, pos_y}, 3.0f, GREEN);
    } else {
        draw_rectangle_lines_ex({left, top, width, height}, 1.0f, YELLOW);
    }
#else
    // In standalone mode, draw with yellow
    draw_rectangle_lines_ex({left, top, width, height}, 1.0f, YELLOW);
#endif
}

bool CCollider::is_point_inside(float px, float py) const {
    // Query CTransform component to get position
    auto transform_opt = Query::try_get<CTransform>(this);
    
    if (!transform_opt) {
        return false; // No transform, can't check
    }
    
    CTransform& transform = transform_opt->get();
    
    // Get position from transform and apply offset
    float pos_x = transform.position_x + offset_x;
    float pos_y = transform.position_y + offset_y;
    
    // Check if point is inside the rectangle (centered at position)
    float left = pos_x - width / 2.0f;
    float right = pos_x + width / 2.0f;
    float top = pos_y - height / 2.0f;
    float bottom = pos_y + height / 2.0f;
    
    return px >= left && px <= right && py >= top && py <= bottom;
}
