#include "game/particle_system.h"
#include "core/raylib_wrapper.h"
#include "core/zeytin.h"

void ParticleSystem::spawn_collision_particles(const Position& pos1, const Position& pos2, Color color, int count) {
    Vector2 collision_center = {
        (pos1.x + pos2.x) * 0.5f,
        (pos1.y + pos2.y) * 0.5f
    };

    Vector2 collision_direction = {
        pos2.x - pos1.x,
        pos2.y - pos1.y
    };

    float dir_length = vector2_length(collision_direction);
    if (dir_length < 0.001f) {
        collision_direction = {1.0f, 0.0f};
    } else {
        collision_direction = vector2_scale(collision_direction, 1.0f / dir_length);
    }

    Vector2 perpendicular = {
        -collision_direction.y,
        collision_direction.x
    };

    int particles_to_spawn = std::min(MAX_PARTICLES - static_cast<int>(m_particles.size()), count);
    
    for (int i = 0; i < particles_to_spawn; ++i) {
        Particle p;

        float angle = get_random_value(0, 628) / 100.0f; 
        float speed_factor = get_random_value(50, 150) / 100.0f;
        float radius = get_random_value(0, 100) / 100.0f * 10.0f;

        p.position = {
            collision_center.x + std::cos(angle) * radius,
            collision_center.y + std::sin(angle) * radius
        };

        p.velocity = {
            std::cos(angle) * PARTICLE_SPEED * speed_factor,
            std::sin(angle) * PARTICLE_SPEED * speed_factor
        };

        p.color = {
            static_cast<unsigned char>(get_random_value(0, 255)),  
            static_cast<unsigned char>(get_random_value(0, 255)), 
            static_cast<unsigned char>(get_random_value(0, 255)),
            200  
        };

        p.lifetime = 0.0f;
        p.max_lifetime = PARTICLE_LIFETIME;

        m_particles.push_back(p);
    }	
}

void ParticleSystem::on_update() {
    for (const auto& particle : m_particles) {
        draw_line_v(
            particle.position, 
            {
                particle.position.x + particle.velocity.x * 0.05f, 
                particle.position.y + particle.velocity.y * 0.05f
            }, 
            particle.color
        );

    }
}

void ParticleSystem::on_play_update() {
    float delta_time = get_frame_time();

    for (auto it = m_particles.begin(); it != m_particles.end();) {
        it->position.x += it->velocity.x * delta_time;
        it->position.y += it->velocity.y * delta_time;
        
        it->lifetime += delta_time;
        
        float alpha_factor = 1.0f - (it->lifetime / it->max_lifetime);
        it->color.a = static_cast<unsigned char>(200 * alpha_factor);
        
        if (it->lifetime >= it->max_lifetime) {
            it = m_particles.erase(it);
        } else {
            ++it;
        }
    }
}

void ParticleSystem::spawn_teleport_effect(const Position& start_pos, const Position& end_pos) {
    for (int i = 0; i < 100; i++) {
        Particle p;

        float angle = GetRandomValue(0, 628) / 100.0f;
        float distance = GetRandomValue(0, 50) / 10.0f;
        float speed = GetRandomValue(100, 300);

        p.position = {
            start_pos.x + cosf(angle) * distance,
            start_pos.y + sinf(angle) * distance
        };

        p.velocity = {
            cosf(angle) * speed,
            sinf(angle) * speed
        };

        p.color = {
            static_cast<unsigned char>(100 + GetRandomValue(0, 155)),
            static_cast<unsigned char>(GetRandomValue(0, 100)),
            static_cast<unsigned char>(100 + GetRandomValue(0, 155)),
            static_cast<unsigned char>(200 + GetRandomValue(0, 55))
        };

        p.lifetime = 0.0f;
        p.max_lifetime = PARTICLE_LIFETIME * 0.7f;

        m_particles.push_back(p);
    }

    for (int i = 0; i < 150; i++) {
        Particle p;

        float angle = GetRandomValue(0, 628) / 100.0f;
        float distance = GetRandomValue(30, 100) / 10.0f;
        float speed = GetRandomValue(50, 200);

        p.position = {
            end_pos.x + cosf(angle) * distance,
            end_pos.y + sinf(angle) * distance
        };

        p.velocity = {
            -cosf(angle) * speed,
            -sinf(angle) * speed
        };

        p.color = {
            static_cast<unsigned char>(100 + GetRandomValue(0, 155)),
            static_cast<unsigned char>(GetRandomValue(0, 100)),
            static_cast<unsigned char>(100 + GetRandomValue(0, 155)),
            static_cast<unsigned char>(200 + GetRandomValue(0, 55))
        };

        p.lifetime = 0.0f;
        p.max_lifetime = PARTICLE_LIFETIME * 0.8f;

        m_particles.push_back(p);
    }
}
