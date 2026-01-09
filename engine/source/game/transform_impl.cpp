module;

#include "raylib.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <cmath>

module zeytin.game.transform;
import zeytin.raylib;
import zeytin.logger;

#ifdef EDITOR_MODE
import zeytin.zeytin;
import zeytin.editor.event;
#endif

void CTransform::on_update() {
#ifdef EDITOR_MODE
    // Draw gizmo and handle interaction if this entity is selected (only in edit mode, not play mode)
    if (!Zeytin::get().is_play_mode() && Zeytin::get().get_selected_entity() == get_id()) {
        handle_gizmo_interaction();
        draw_gizmo();
    }
#endif
}

void CTransform::draw_gizmo() {
    const float axis_length = 100.0f;
    const float arrow_size = 15.0f;
    const float line_thickness = 4.0f;
    
    // X axis (red) - pointing right
    draw_line_ex({position_x, position_y}, {position_x + axis_length, position_y}, line_thickness, RED);
    draw_triangle({position_x + axis_length, position_y}, 
                  {position_x + axis_length - arrow_size, position_y - arrow_size/2}, 
                  {position_x + axis_length - arrow_size, position_y + arrow_size/2}, RED);
    
    // Y axis UP (blue) - pointing up for clarity
    draw_line_ex({position_x, position_y}, {position_x, position_y - axis_length}, line_thickness, BLUE);
    draw_triangle({position_x, position_y - axis_length}, 
                  {position_x - arrow_size/2, position_y - axis_length + arrow_size}, 
                  {position_x + arrow_size/2, position_y - axis_length + arrow_size}, BLUE);
    
    // Center point
    draw_circle_v({position_x, position_y}, 8.0f, YELLOW);
}

#ifdef EDITOR_MODE
void CTransform::handle_gizmo_interaction() {
    Vector2 mouse_pos = get_mouse_position();
    Vector2 world_mouse = get_screen_to_world2d(mouse_pos, Zeytin::get().get_camera());
    
    // Start dragging - check center first, then axes
    if (is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
        if (is_hovering_center(world_mouse.x, world_mouse.y)) {
            // Drag from center - free 2D movement
            m_dragging_axis = DragAxis::Both;
            m_drag_start_x = world_mouse.x;
            m_drag_start_y = world_mouse.y;
            m_drag_start_position_x = position_x;
            m_drag_start_position_y = position_y;
        } else if (is_hovering_axis(world_mouse.x, world_mouse.y, true)) {
            // Drag X axis - horizontal only
            m_dragging_axis = DragAxis::X;
            m_drag_start_x = world_mouse.x;
            m_drag_start_y = world_mouse.y;
            m_drag_start_position_x = position_x;
            m_drag_start_position_y = position_y;
        } else if (is_hovering_axis(world_mouse.x, world_mouse.y, false)) {
            // Drag Y axis - vertical only
            m_dragging_axis = DragAxis::Y;
            m_drag_start_x = world_mouse.x;
            m_drag_start_y = world_mouse.y;
            m_drag_start_position_x = position_x;
            m_drag_start_position_y = position_y;
        }
    }
    
    // During drag - update position locally
    if (m_dragging_axis != DragAxis::None && is_mouse_button_down(MOUSE_BUTTON_LEFT)) {
        if (m_dragging_axis == DragAxis::X) {
            float delta_x = world_mouse.x - m_drag_start_x;
            position_x = m_drag_start_position_x + delta_x;
        } else if (m_dragging_axis == DragAxis::Y) {
            float delta_y = world_mouse.y - m_drag_start_y;
            position_y = m_drag_start_position_y + delta_y;
        } else if (m_dragging_axis == DragAxis::Both) {
            float delta_x = world_mouse.x - m_drag_start_x;
            float delta_y = world_mouse.y - m_drag_start_y;
            position_x = m_drag_start_position_x + delta_x;
            position_y = m_drag_start_position_y + delta_y;
        }
    }
    
    // End dragging - send command(s) to editor
    if (m_dragging_axis != DragAxis::None && is_mouse_button_released(MOUSE_BUTTON_LEFT)) {
        if (m_dragging_axis == DragAxis::X) {
            send_property_change_command("position_x", m_drag_start_position_x, position_x);
        } else if (m_dragging_axis == DragAxis::Y) {
            send_property_change_command("position_y", m_drag_start_position_y, position_y);
        } else if (m_dragging_axis == DragAxis::Both) {
            // Send batch property change for both X and Y
            send_batch_property_change_command();
        }
        m_dragging_axis = DragAxis::None;
    }
}

bool CTransform::is_hovering_axis(float mouse_x, float mouse_y, bool is_x_axis) const {
    const float axis_length = 100.0f;
    const float hover_threshold = 10.0f;
    
    if (is_x_axis) {
        // Check if mouse is near X axis line
        float dist_from_line = std::abs(mouse_y - position_y);
        return dist_from_line < hover_threshold && 
               mouse_x >= position_x && 
               mouse_x <= position_x + axis_length;
    } else {
        // Check if mouse is near Y axis line (pointing up)
        float dist_from_line = std::abs(mouse_x - position_x);
        return dist_from_line < hover_threshold && 
               mouse_y <= position_y && 
               mouse_y >= position_y - axis_length;
    }
}

bool CTransform::is_hovering_center(float mouse_x, float mouse_y) const {
    const float center_radius = 8.0f; // Same as the yellow circle
    const float hover_radius = 12.0f; // Slightly larger for easier clicking
    
    float dx = mouse_x - position_x;
    float dy = mouse_y - position_y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    return distance <= hover_radius;
}

void CTransform::send_property_change_command(const char* property_name, float old_value, float new_value) {
    rapidjson::Document msg;
    msg.SetObject();
    auto& alloc = msg.GetAllocator();
    
    msg.AddMember("type", "property_change_command", alloc);
    msg.AddMember("entity_id", get_id(), alloc);
    msg.AddMember("variant_type", "CTransform", alloc);
    
    rapidjson::Value prop_name(property_name, alloc);
    msg.AddMember("key_path", prop_name, alloc);
    msg.AddMember("old_value", old_value, alloc);
    msg.AddMember("new_value", new_value, alloc);
    
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);
    
    EditorEventBus::get().publish<std::string>(EditorEvent::SyncEditor, buffer.GetString());
    
    log_info("Sent property change command: {} = {} -> {}", property_name, old_value, new_value);
}

void CTransform::send_batch_property_change_command() {
    rapidjson::Document msg;
    msg.SetObject();
    auto& alloc = msg.GetAllocator();
    
    msg.AddMember("type", "batch_property_change_command", alloc);
    msg.AddMember("entity_id", get_id(), alloc);
    msg.AddMember("variant_type", "CTransform", alloc);
    
    // Create changes array
    rapidjson::Value changes(rapidjson::kArrayType);
    
    // Add position_x change
    {
        rapidjson::Value change(rapidjson::kObjectType);
        change.AddMember("key_path", "position_x", alloc);
        change.AddMember("old_value", m_drag_start_position_x, alloc);
        change.AddMember("new_value", position_x, alloc);
        changes.PushBack(change, alloc);
    }
    
    // Add position_y change
    {
        rapidjson::Value change(rapidjson::kObjectType);
        change.AddMember("key_path", "position_y", alloc);
        change.AddMember("old_value", m_drag_start_position_y, alloc);
        change.AddMember("new_value", position_y, alloc);
        changes.PushBack(change, alloc);
    }
    
    msg.AddMember("changes", changes, alloc);
    
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);
    
    EditorEventBus::get().publish<std::string>(EditorEvent::SyncEditor, buffer.GetString());
    
    log_info("Sent batch property change command: position_x ({} -> {}), position_y ({} -> {})", 
             m_drag_start_position_x, position_x, m_drag_start_position_y, position_y);
}
#endif
