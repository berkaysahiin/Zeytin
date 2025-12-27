#include "game/objective_arrow.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<ObjectiveArrow>("ObjectiveArrow")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("arrow_height", &ObjectiveArrow::arrow_height)
        .property("arrow_size", &ObjectiveArrow::arrow_size)
        .property("bounce_amount", &ObjectiveArrow::bounce_amount)
        .property("bounce_speed", &ObjectiveArrow::bounce_speed);
}
