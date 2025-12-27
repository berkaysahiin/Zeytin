#include "game/test_variant.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<TestVariant>("TestVariant")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("can_move", &TestVariant::can_move)
        .property("gravity", &TestVariant::gravity)
        .property("health", &TestVariant::health)
        .property("is_active", &TestVariant::is_active)
        .property("is_invincible", &TestVariant::is_invincible)
        .property("jump_force", &TestVariant::jump_force)
        .property("level", &TestVariant::level)
        .property("max_health", &TestVariant::max_health)
        .property("name", &TestVariant::name)
        .property("speed", &TestVariant::speed)
        .property("tag", &TestVariant::tag);
}
