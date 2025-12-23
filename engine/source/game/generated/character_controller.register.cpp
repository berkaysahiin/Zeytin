#include "rttr/registration.h"
#include "game/character_controller.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<CharacterController>("CharacterController")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("acceleration", &CharacterController::acceleration)
        .property("friction", &CharacterController::friction);
}

