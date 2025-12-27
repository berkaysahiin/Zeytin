#include "game/background.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Background>("Background")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("bg_color", &Background::bg_color)
        .property("star_count", &Background::star_count)
        .property("star_speed_max", &Background::star_speed_max)
        .property("star_speed_min", &Background::star_speed_min);
}
