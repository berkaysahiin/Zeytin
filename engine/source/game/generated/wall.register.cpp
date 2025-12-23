#include "rttr/registration.h"
#include "game/wall.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Wall>("Wall")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);
}

