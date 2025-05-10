#include "game/particle_system.h"
#include "core/raylib_wrapper.h"
#include "core/zeytin.h"

void ParticleSystem::spawn_collision_particles(const Position& pos1, const Position& pos2, Color color1, Color color2) {
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

    int particles_to_spawn = std::min(MAX_PARTICLES - static_cast<int>(m_particles.size()), 50);
    
    for (int i = 0; i < particles_to_spawn; ++i) {
        Particle p;
        
        float spread_factor = get_random_value(-100, 100) / 100.0f;
        float line_factor = get_random_value(-100, 100) / 100.0f;
        
        p.position = {
            collision_center.x + perpendicular.x * spread_factor * 20.0f + 
                        collision_direction.x * line_factor * 20.0f,
            collision_center.y + perpendicular.y * spread_factor * 20.0f + 
                        collision_direction.y * line_factor * 20.0f
        };

        float speed_factor = get_random_value(50, 150) / 100.0f;
        p.velocity = {
            collision_direction.x * PARTICLE_SPEED * speed_factor,
            collision_direction.y * PARTICLE_SPEED * speed_factor
        };

        float color_factor = get_random_value(0, 100) / 100.0f;
        p.color = {
            static_cast<unsigned char>((color1.r * (1.0f - color_factor) + color2.r * color_factor)),
            static_cast<unsigned char>((color1.g * (1.0f - color_factor) + color2.g * color_factor)),
            static_cast<unsigned char>((color1.b * (1.0f - color_factor) + color2.b * color_factor)),
            static_cast<unsigned char>(200)
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
