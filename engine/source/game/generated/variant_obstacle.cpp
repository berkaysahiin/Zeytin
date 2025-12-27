#include "game/obstacle.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Obstacle>("Obstacle")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("outline_thickness", &Obstacle::outline_thickness)
        .property("show_outline", &Obstacle::show_outline);
}
