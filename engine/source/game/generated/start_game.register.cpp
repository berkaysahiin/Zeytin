#include "rttr/registration.h"
#include "game/start_game.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<StartGame>("StartGame")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("background_opacity", &StartGame::background_opacity)
        .property("controls_font_size", &StartGame::controls_font_size)
        .property("display_duration", &StartGame::display_duration)
        .property("fade_in_duration", &StartGame::fade_in_duration)
        .property("game_title", &StartGame::game_title)
        .property("instruction_font_size", &StartGame::instruction_font_size)
        .property("instruction_text", &StartGame::instruction_text)
        .property("show_controls", &StartGame::show_controls)
        .property("show_instructions", &StartGame::show_instructions)
        .property("title_font_size", &StartGame::title_font_size);
}

