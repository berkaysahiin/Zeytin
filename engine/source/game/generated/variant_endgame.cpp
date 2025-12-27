#include "game/end_game.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<EndGame>("EndGame")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("game_over_font_size", &EndGame::game_over_font_size)
        .property("next_level", &EndGame::next_level)
        .property("reason_font_size", &EndGame::reason_font_size)
        .property("ui_transparency", &EndGame::ui_transparency)
        .property("win_font_size", &EndGame::win_font_size);
}
