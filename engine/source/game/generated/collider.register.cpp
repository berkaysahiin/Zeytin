#include "rttr/registration.h"
#include "game/collider.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Collider>("Collider")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("m_collider_type", &Collider::m_collider_type)
        .property("m_draw_debug", &Collider::m_draw_debug)
        .property("m_height", &Collider::m_height)
        .property("m_is_trigger", &Collider::m_is_trigger)
        .property("m_radius", &Collider::m_radius)
        .property("m_static", &Collider::m_static)
        .property("m_width", &Collider::m_width)
        .property("thickness", &Collider::thickness);
}

