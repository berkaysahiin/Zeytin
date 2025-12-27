#include "game/diffuser.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Diffuser>("Diffuser")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("bob_amount", &Diffuser::bob_amount)
        .property("bob_speed", &Diffuser::bob_speed)
        .property("icon_offset_y", &Diffuser::icon_offset_y)
        .property("icon_size", &Diffuser::icon_size)
        .property("pickup_radius", &Diffuser::pickup_radius);
}
