#include "game/bullet_config.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<BulletConfig>("BulletConfig")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("airtime_threshold", &BulletConfig::airtime_threshold)
        .property("height", &BulletConfig::height)
        .property("lifetime", &BulletConfig::lifetime)
        .property("speed", &BulletConfig::speed)
        .property("width", &BulletConfig::width);
}
