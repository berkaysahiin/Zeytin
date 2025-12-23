#include "rttr/registration.h"
#include "game/powerup.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<PowerUp>("PowerUp")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);
}

