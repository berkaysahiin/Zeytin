#include "rttr/registration.h"
#include "game/player_renderer.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<PlayerRenderer>("PlayerRenderer")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("corner_radius", &PlayerRenderer::corner_radius)
        .property("corner_segments", &PlayerRenderer::corner_segments)
        .property("m_max_history_length", &PlayerRenderer::m_max_history_length)
        .property("m_trail_fade_factor", &PlayerRenderer::m_trail_fade_factor)
        .property("outline_thickness", &PlayerRenderer::outline_thickness)
        .property("player_label_size", &PlayerRenderer::player_label_size)
        .property("pulse_intensity", &PlayerRenderer::pulse_intensity)
        .property("pulse_speed", &PlayerRenderer::pulse_speed)
        .property("show_player_number", &PlayerRenderer::show_player_number)
        .property("show_trail", &PlayerRenderer::show_trail)
        .property("trail_thickness", &PlayerRenderer::trail_thickness)
        .property("use_outline", &PlayerRenderer::use_outline);
}

