#include "game/time_stopped_indicator.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<TimeStoppedIndicator>("TimeStoppedIndicator")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("activate_threshhold", &TimeStoppedIndicator::activate_threshhold)
        .property("enabled", &TimeStoppedIndicator::enabled)
        .property("font_size", &TimeStoppedIndicator::font_size)
        .property("padding", &TimeStoppedIndicator::padding);
}
