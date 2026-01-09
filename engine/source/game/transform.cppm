module;

#include "preparser.h"
#include <cstdint>

export module zeytin.game.transform;
import zeytin.component;
import zeytin.entity;

export struct CTransform : public Component 
{
    CTransform() = default; // Required default constructor

    // Lifecycle methods
    void on_update() override;

    // Gizmo drawing and manipulation
    void draw_gizmo();
    void handle_gizmo_interaction();
    
    // Helper functions
    bool is_hovering_axis(float mouse_x, float mouse_y, bool is_x_axis) const;
    bool is_hovering_center(float mouse_x, float mouse_y) const;
    void send_property_change_command(const char* property_name, float old_value, float new_value);
    void send_batch_property_change_command();

    // Position
    PROPERTY(GROUP="Position")
    float position_x = 0.0f;

    PROPERTY(GROUP="Position")
    float position_y = 0.0f;

    // Rotation (in degrees)
    PROPERTY(GROUP="Rotation")
    float rotation = 0.0f;

    // Scale
    PROPERTY(GROUP="Scale")
    float scale_x = 1.0f;

    PROPERTY(GROUP="Scale")
    float scale_y = 1.0f;

    // Hierarchy - Parent entity ID for hierarchical transforms
    // Note: Not exposed via PROPERTY to avoid serialization issues
    uint64_t parent_id = 0;

private:
    // Gizmo manipulation state
    enum class DragAxis { None, X, Y, Both };
    DragAxis m_dragging_axis = DragAxis::None;
    float m_drag_start_x = 0.0f;
    float m_drag_start_y = 0.0f;
    float m_drag_start_position_x = 0.0f;
    float m_drag_start_position_y = 0.0f;
};

