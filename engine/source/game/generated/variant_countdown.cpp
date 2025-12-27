#include "game/countdown.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Countdown>("Countdown")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("countdown_active", &Countdown::countdown_active)
        .property("danger_threshold", &Countdown::danger_threshold)
        .property("duration", &Countdown::duration)
        .property("font_size", &Countdown::font_size)
        .property("offset_x", &Countdown::offset_x)
        .property("offset_y", &Countdown::offset_y)
        .property("pause_when_zero", &Countdown::pause_when_zero)
        .property("warning_threshold", &Countdown::warning_threshold);
}
