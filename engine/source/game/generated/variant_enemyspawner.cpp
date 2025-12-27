#include "game/enemy_spawner.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<EnemySpawner>("EnemySpawner")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("enemy_type", &EnemySpawner::enemy_type)
        .property("initial_enemy_count", &EnemySpawner::initial_enemy_count)
        .property("initial_spawn_delay", &EnemySpawner::initial_spawn_delay)
        .property("keep_spawning", &EnemySpawner::keep_spawning)
        .property("max_alive_at_once", &EnemySpawner::max_alive_at_once)
        .property("min_distance_between_enemies", &EnemySpawner::min_distance_between_enemies)
        .property("player_safe_zone_radius", &EnemySpawner::player_safe_zone_radius)
        .property("show_debug_spawner", &EnemySpawner::show_debug_spawner)
        .property("spawn_area_center_x", &EnemySpawner::spawn_area_center_x)
        .property("spawn_area_center_y", &EnemySpawner::spawn_area_center_y)
        .property("spawn_area_height", &EnemySpawner::spawn_area_height)
        .property("spawn_area_width", &EnemySpawner::spawn_area_width)
        .property("spawn_every_seconds", &EnemySpawner::spawn_every_seconds);
}
