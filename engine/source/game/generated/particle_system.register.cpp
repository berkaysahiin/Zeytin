#include "rttr/registration.h"
#include "game/particle_system.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<ParticleSystem>("ParticleSystem")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("MAX_PARTICLES", &ParticleSystem::MAX_PARTICLES)
        .property("PARTICLE_LIFETIME", &ParticleSystem::PARTICLE_LIFETIME)
        .property("PARTICLE_SPEED", &ParticleSystem::PARTICLE_SPEED);
}

