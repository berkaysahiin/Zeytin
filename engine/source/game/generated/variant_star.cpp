#include "game/background.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Star>("Star")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("brightness", &Star::brightness)
        .property("size", &Star::size)
        .property("speed", &Star::speed)
        .property("x", &Star::x)
        .property("y", &Star::y)
        (rttr::metadata("NO_VARIANT", true));
}
