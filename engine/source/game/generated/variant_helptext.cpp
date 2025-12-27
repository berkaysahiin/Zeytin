#include "game/help_text.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<HelpText>("HelpText")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("auto_destroy_after_fade", &HelpText::auto_destroy_after_fade)
        .property("background_border_radius", &HelpText::background_border_radius)
        .property("background_color", &HelpText::background_color)
        .property("background_padding", &HelpText::background_padding)
        .property("center_horizontally", &HelpText::center_horizontally)
        .property("center_on_screen", &HelpText::center_on_screen)
        .property("center_vertically", &HelpText::center_vertically)
        .property("display_duration", &HelpText::display_duration)
        .property("fade_in_duration", &HelpText::fade_in_duration)
        .property("fade_out_duration", &HelpText::fade_out_duration)
        .property("font_size", &HelpText::font_size)
        .property("show_background", &HelpText::show_background)
        .property("text", &HelpText::text)
        .property("text_color", &HelpText::text_color);
}
