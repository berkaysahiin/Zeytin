#include "rttr/registration.h"
#include "game/zone.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Zone>("Zone")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);
}

