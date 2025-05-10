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

    bool show_trail = true; PROPERTY();
    int m_max_history_length = 10; PROPERTY();
    float m_trail_fade_factor = 0.9f; PROPERTY();
    float trail_thickness = 10; PROPERTY();

    Color player_color;
    
    virtual void on_init() override;
    virtual void on_update() override;
    
private:
    void draw_player();
    void draw_trail_effect();
    void update_position_history();

    std::vector<Vector2> m_position_history;
};
