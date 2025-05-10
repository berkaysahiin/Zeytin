#include "game/powerup_spawner.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "core/zeytin.h"
#include "game/position.h"
#include "game/collider.h"
#include "game/powerup.h"

#include "remote_logger/remote_logger.h"

PowerUp::Type PowerUpSpawner::get_random_type() {
    int type_count = static_cast<int>(PowerUp::Type::LENGTH);
    int random_index = get_random_value(1, type_count - 1); 
    return static_cast<PowerUp::Type>(random_index);
}

::entity_id PowerUpSpawner::spawn_powerup(float x, float y, PowerUp::Type type) {
    auto powerup_entity = Query::create_entity();

    Query::add<Position>(powerup_entity, x, y);
    
    auto& collider = Query::add<Collider>(powerup_entity).value().get();
    collider.m_collider_type = 2; 
    collider.m_radius = m_spawn_radius;
    collider.m_draw_debug = m_show_debug_visuals;
    
    auto& powerup = Query::add<PowerUp>(powerup_entity).value().get();
    powerup.m_type = type;
    
    m_time_until_next_spawn = m_spawn_interval;
              
    return powerup_entity;
}

void PowerUpSpawner::on_play_start() {
    m_time_until_next_spawn = m_spawn_interval;
}

void PowerUpSpawner::on_play_update() {
    m_time_until_next_spawn -= get_frame_time();

    if (m_time_until_next_spawn <= 0) {
        float x = get_random_value(m_spawn_radius, VIRTUAL_WIDTH - m_spawn_radius);
        float y = get_random_value(m_spawn_radius, VIRTUAL_HEIGHT - m_spawn_radius);
        
        PowerUp::Type type = m_randomize_type ? get_random_type() : m_fixed_type;
        
        spawn_powerup(x, y, type);
    }
}
