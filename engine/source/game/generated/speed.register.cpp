#include "rttr/registration.h"
#include "game/speed.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Speed>("Speed")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("value", &Speed::value);
}

