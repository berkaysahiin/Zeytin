#pragma once

#include "variant/variant_base.h"
#include "game/position.h"

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float lifetime;
    float max_lifetime;
};

class ParticleSystem : public VariantBase {
    VARIANT(ParticleSystem);

public:
    void spawn_collision_particles(const Position& pos1, const Position& pos2, Color color, int count=100);
    void spawn_teleport_effect(const Position& start_pos, const Position& end_pos);
    
    virtual void on_update() override;
    virtual void on_play_update() override;

    int MAX_PARTICLES = 10000; PROPERTY();
    float PARTICLE_LIFETIME = 0.5f; PROPERTY();
    float PARTICLE_SPEED = 200.0f; PROPERTY();

private:
    std::vector<Particle> m_particles;
};
