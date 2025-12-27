#include "game/enemy.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Enemy>("Enemy")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("body_size", &Enemy::body_size)
        .property("death_fade_duration", &Enemy::death_fade_duration)
        .property("eye_offset_x", &Enemy::eye_offset_x)
        .property("eye_offset_y", &Enemy::eye_offset_y)
        .property("eye_size", &Enemy::eye_size)
        .property("eye_spacing", &Enemy::eye_spacing)
        .property("gravity", &Enemy::gravity)
        .property("gun_length", &Enemy::gun_length)
        .property("gun_offset_x", &Enemy::gun_offset_x)
        .property("gun_offset_y", &Enemy::gun_offset_y)
        .property("gun_width", &Enemy::gun_width)
        .property("max_fall_speed", &Enemy::max_fall_speed)
        .property("patrol_distance", &Enemy::patrol_distance)
        .property("patrol_speed", &Enemy::patrol_speed)
        .property("shoot_interval", &Enemy::shoot_interval)
        .property("shoot_range", &Enemy::shoot_range)
        .property("use_global_config", &Enemy::use_global_config);
}
