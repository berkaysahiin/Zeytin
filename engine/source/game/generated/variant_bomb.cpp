#include "game/bomb.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Bomb>("Bomb")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("bar_border_thickness", &Bomb::bar_border_thickness)
        .property("bar_height", &Bomb::bar_height)
        .property("bar_offset_y", &Bomb::bar_offset_y)
        .property("bar_outline_thickness", &Bomb::bar_outline_thickness)
        .property("bar_progress_font_size", &Bomb::bar_progress_font_size)
        .property("bar_text_offset_y", &Bomb::bar_text_offset_y)
        .property("bar_text_shadow_offset", &Bomb::bar_text_shadow_offset)
        .property("bar_width", &Bomb::bar_width)
        .property("bomb_size", &Bomb::bomb_size)
        .property("defuse_radius", &Bomb::defuse_radius)
        .property("defuse_time", &Bomb::defuse_time)
        .property("glow_size", &Bomb::glow_size)
        .property("label_size", &Bomb::label_size)
        .property("pulse_intensity", &Bomb::pulse_intensity)
        .property("pulse_speed", &Bomb::pulse_speed)
        .property("ring_speed", &Bomb::ring_speed)
        .property("shake_intensity", &Bomb::shake_intensity)
        .property("show_danger_rings", &Bomb::show_danger_rings);
}
