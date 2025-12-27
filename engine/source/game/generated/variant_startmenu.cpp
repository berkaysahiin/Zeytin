#include "game/start_menu.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<StartMenu>("StartMenu")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("background_opacity", &StartMenu::background_opacity)
        .property("instruction_font_size", &StartMenu::instruction_font_size)
        .property("next_level", &StartMenu::next_level)
        .property("title_font_size", &StartMenu::title_font_size);
}
