#include "game/collider.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Collider>("Collider")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("height", &Collider::height)
        .property("show_debug", &Collider::show_debug)
        .property("width", &Collider::width);
}
