#pragma once

#include "variant/variant_base.h"
#include "entity/entity.h"
#include "game/powerup.h"

class PowerUpSpawner : public VariantBase {
    VARIANT(PowerUpSpawner);

public:
    ::entity_id spawn_powerup(float x, float y, PowerUp::Type type);
    
    virtual void on_play_start() override;
    virtual void on_play_update() override;

    float m_spawn_interval = 5.0f; PROPERTY()
    float m_min_lifetime = 5.0f; PROPERTY()
    float m_max_lifetime = 15.0f; PROPERTY()
    bool m_randomize_type = true; PROPERTY()
    PowerUp::Type m_fixed_type = PowerUp::Type::SPEED_BOOST; PROPERTY()
    float m_spawn_radius = 50.0f; PROPERTY()
    bool m_show_debug_visuals = true; PROPERTY()

private:
    float m_time_until_next_spawn = 0;
    
    PowerUp::Type get_random_type();
};
