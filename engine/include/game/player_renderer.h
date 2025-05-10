#pragma once

#include "variant/variant_base.h"
#include "game/position.h"
#include "game/player_info.h"
#include "game/scale.h"

class PlayerRenderer : public VariantBase {
    VARIANT(PlayerRenderer);
    REQUIRES(PlayerInfo, Collider)

public:
    bool use_outline = true; PROPERTY()
    float outline_thickness = 2.0f; PROPERTY()
    float corner_radius = 8.0f; PROPERTY()
    int corner_segments = 10; PROPERTY()
    
    bool show_player_number = true; PROPERTY()
    int player_label_size = 20; PROPERTY()
    
    float pulse_intensity = 0.05f; PROPERTY()
    float pulse_speed = 5.0f; PROPERTY()
    
    bool show_trail = true; PROPERTY()
    float trail_speed = 0.5f; PROPERTY()
    float trail_length = 1.0f; PROPERTY()
    int trail_count = 3; PROPERTY()
    float trail_thickness = 2.0f; PROPERTY()
    Color trail_color = WHITE; PROPERTY()
    
    bool show_zone_effect = true; PROPERTY()
    float zone_effect_size = 1.3f; PROPERTY()
    float zone_effect_speed = 8.0f; PROPERTY()
    float zone_effect_duration = 10.0f; PROPERTY()
    
    virtual void on_init() override;
    virtual void on_update() override;
    
private:
    void draw_player();
    void draw_zone_effect(const Vector2& position, float width, float height, float zone_time);
    void draw_trail_effect(const Vector2& position, float width, float height);
};
