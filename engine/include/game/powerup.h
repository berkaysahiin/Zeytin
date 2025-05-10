#pragma once

#include "variant/variant_base.h"
#include "game/position.h"
#include "game/collider.h"

class PowerUp : public VariantBase {
    VARIANT(PowerUp);
    REQUIRES(Position, Collider)

public:
    enum class Type {
        NONE,
        SPEED_BOOST,
        SUPER_SIZE,  
        SHRINK,       
        ZONE_SLOW,     
        SHIELD,
        LENGTH,
    };

    Type type = Type::SPEED_BOOST; PROPERTY()
    float power_multiplier = 1.5f; PROPERTY() 
    float duration = 5.0f; PROPERTY() 
    float m_lifetime = 15.0f; PROPERTY() 
    float m_since_spawn = 0.0f; PROPERTY() 

    virtual void on_post_init() override;
    virtual void on_update() override;  
    virtual void on_play_update() override;

private:
    void handle_player_collision(Collider& other);
    void apply_effect_to_player(int player_index);

    void draw_speed_boost(const Position& position, float pulse, float time);
    void draw_super_size(const Position& position, float pulse, float time);
    void draw_shrink(const Position& position, float pulse, float time);  
    void draw_zone_slow(const Position& position, float pulse, float time);
    void draw_shield(const Position& position, float pulse, float time);
    void draw_expiration_warning(const Position& position, float time, float base_radius);
};
