#include "game/enemy_config.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<EnemyConfig>("EnemyConfig")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("body_size", &EnemyConfig::body_size)
        .property("death_fade_duration", &EnemyConfig::death_fade_duration)
        .property("eye_offset_x", &EnemyConfig::eye_offset_x)
        .property("eye_offset_y", &EnemyConfig::eye_offset_y)
        .property("eye_size", &EnemyConfig::eye_size)
        .property("eye_spacing", &EnemyConfig::eye_spacing)
        .property("gravity", &EnemyConfig::gravity)
        .property("gun_length", &EnemyConfig::gun_length)
        .property("gun_offset_x", &EnemyConfig::gun_offset_x)
        .property("gun_offset_y", &EnemyConfig::gun_offset_y)
        .property("gun_width", &EnemyConfig::gun_width)
        .property("max_fall_speed", &EnemyConfig::max_fall_speed)
        .property("patrol_distance", &EnemyConfig::patrol_distance)
        .property("patrol_speed", &EnemyConfig::patrol_speed)
        .property("shoot_interval", &EnemyConfig::shoot_interval)
        .property("shoot_range", &EnemyConfig::shoot_range);
}
