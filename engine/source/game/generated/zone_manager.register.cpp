#include "rttr/registration.h"
#include "game/zone_manager.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<ZoneManager>("ZoneManager")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("m_max_radius", &ZoneManager::m_max_radius)
        .property("m_max_secs", &ZoneManager::m_max_secs)
        .property("m_min_radius", &ZoneManager::m_min_radius)
        .property("m_min_secs", &ZoneManager::m_min_secs)
        .property("m_spawn_every_secs", &ZoneManager::m_spawn_every_secs)
        .property("m_thick_incess_rate", &ZoneManager::m_thick_incess_rate)
        .property("show_debug_visuals", &ZoneManager::show_debug_visuals);
}

