#include "game/bullet.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Bullet>("Bullet")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);
}
