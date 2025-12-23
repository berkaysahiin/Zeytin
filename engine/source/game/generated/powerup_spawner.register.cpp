#include "rttr/registration.h"
#include "game/powerup_spawner.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<PowerUpSpawner>("PowerUpSpawner")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("m_max_lifetime", &PowerUpSpawner::m_max_lifetime)
        .property("m_min_lifetime", &PowerUpSpawner::m_min_lifetime)
        .property("m_randomize_type", &PowerUpSpawner::m_randomize_type)
        .property("m_show_debug_visuals", &PowerUpSpawner::m_show_debug_visuals)
        .property("m_spawn_interval", &PowerUpSpawner::m_spawn_interval)
        .property("m_spawn_radius", &PowerUpSpawner::m_spawn_radius);
}

