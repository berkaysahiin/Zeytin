#include "rttr/registration.h"
#include "game/player_info.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<PlayerInfo>("PlayerInfo")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("color", &PlayerInfo::color)
        .property("critical_threshold", &PlayerInfo::critical_threshold)
        .property("danger_threshold", &PlayerInfo::danger_threshold)
        .property("in_zone", &PlayerInfo::in_zone)
        .property("index", &PlayerInfo::index)
        .property("m_padding", &PlayerInfo::m_padding)
        .property("m_panel_height", &PlayerInfo::m_panel_height)
        .property("m_panel_width", &PlayerInfo::m_panel_width)
        .property("m_panel_x", &PlayerInfo::m_panel_x)
        .property("m_panel_y", &PlayerInfo::m_panel_y)
        .property("max_time_outside", &PlayerInfo::max_time_outside)
        .property("max_zone_time", &PlayerInfo::max_zone_time)
        .property("name", &PlayerInfo::name)
        .property("shield", &PlayerInfo::shield)
        .property("time_spent_zone", &PlayerInfo::time_spent_zone)
        .property("warning_threshold", &PlayerInfo::warning_threshold);
}

