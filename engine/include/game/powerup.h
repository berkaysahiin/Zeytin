#pragma once

#include <cstdint>
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
        TELEPORTER,
        LENGTH,
    };

    Type m_type = Type::SPEED_BOOST; 
    float m_power_multiplier = 1.5f;
    float m_duration = 5.0f; 
    float m_lifetime = 15.0f; 
    float m_since_spawn = 0.0f; 
    float m_since_used = 0;
    uint64_t m_player_id = -1;
    bool m_used = false;

    uint64_t m_linked_teleporter_id = 0; 
    bool m_is_exit_only = false;        
    float m_cooldown = 1.5f;           
    float m_cooldown_remaining = 0.0f;

    virtual void on_post_init() override;
    virtual void on_update() override;  
    virtual void on_play_update() override;

private:
    void consume();
    void handle_player_collision(Collider& other);
    void apply_effect_to_player(int player_index);
    void teleport_player(uint64_t entity);
    void spawn_teleport_effect(const Position& start_pos, const Position& end_pos);

    void teleporter_look_others();

    void draw_speed_boost(const Position& position, float pulse, float time);
    void draw_super_size(const Position& position, float pulse, float time);
    void draw_shrink(const Position& position, float pulse, float time);  
    void draw_zone_slow(const Position& position, float pulse, float time);
    void draw_shield(const Position& position, float pulse, float time);
    void draw_expiration_warning(const Position& position, float time, float base_radius);
    void draw_teleporter(const Position& pos, float pulse, float time);
};
