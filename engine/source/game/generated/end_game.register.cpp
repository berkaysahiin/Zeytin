#include "rttr/registration.h"
#include "game/end_game.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<EndGame>("EndGame")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("countdown_font_size", &EndGame::countdown_font_size)
        .property("game_over_font_size", &EndGame::game_over_font_size)
        .property("post_game_delay", &EndGame::post_game_delay)
        .property("results_font_size", &EndGame::results_font_size)
        .property("show_countdown", &EndGame::show_countdown)
        .property("show_winner_screen", &EndGame::show_winner_screen)
        .property("ui_transparency", &EndGame::ui_transparency);
}

