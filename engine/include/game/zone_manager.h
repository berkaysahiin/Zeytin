#pragma once

#include "variant/variant_base.h"
#include "entity/entity.h"

class ZoneManager : public VariantBase {
    VARIANT(ZoneManager);

public:
    ::entity_id spawn_zone(float x, float y, float radius, float vanish_after_secs);
    
    virtual void on_play_start() override;
    virtual void on_play_update() override;

    float m_spawn_every_secs = 0; PROPERTY();
    float m_min_radius = 0; PROPERTY(); 
    float m_max_radius = 0; PROPERTY();
    float m_thick_incess_rate = 0; PROPERTY();

    float m_min_secs = 0; PROPERTY();
    float m_max_secs = 0; PROPERTY();

    bool show_debug_visuals = true; PROPERTY();

private:
    float m_until_next_spawn = 0;
};
