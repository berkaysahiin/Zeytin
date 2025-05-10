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
    float outline_thickness = 3.0f; PROPERTY()
    float corner_radius = 8.0f; PROPERTY()  
    bool show_player_number = true; PROPERTY()
    
    virtual void on_init() override;
    virtual void on_update() override;
    
private:
    void draw_player();
    void draw_player_outline();
    void draw_player_effects();
};
