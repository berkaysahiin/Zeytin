#include "game/retry.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Retry>("Retry")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("button_font_size", &Retry::button_font_size)
        .property("button_height", &Retry::button_height)
        .property("button_width", &Retry::button_width)
        .property("button_x", &Retry::button_x)
        .property("button_y", &Retry::button_y)
        .property("text_color", &Retry::text_color);
}
