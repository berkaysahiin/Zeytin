#pragma once

#include "variant/variant_base.h"

class Zone : public VariantBase {
    VARIANT(Zone);
    REQUIRES(Collider);

public:
    inline Zone(float vanish_after) : vanish_after_secs(vanish_after) {}

    virtual void on_init() override;
    virtual void on_play_update() override;
    virtual void on_play_late_update() override;

public:
    float vanish_after_secs = 0; PROPERTY();
    float since_spawn_secs = 0; PROPERTY();

private:
    Color get_zone_color() const;

private:
    float m_max_radius = 0; // comes from manager
    float m_min_radius = 0; // comes from manager
    float m_tick_increase = 0; // comes from manager
};
