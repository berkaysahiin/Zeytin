#include "game/rewind_effect.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<RewindEffect>("RewindEffect")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("arrow_bg_color", &RewindEffect::arrow_bg_color)
        .property("arrow_color", &RewindEffect::arrow_color)
        .property("arrow_count", &RewindEffect::arrow_count)
        .property("arrow_size", &RewindEffect::arrow_size)
        .property("arrow_spacing", &RewindEffect::arrow_spacing)
        .property("enabled", &RewindEffect::enabled)
        .property("pulse_intensity", &RewindEffect::pulse_intensity)
        .property("pulse_speed", &RewindEffect::pulse_speed);
}
