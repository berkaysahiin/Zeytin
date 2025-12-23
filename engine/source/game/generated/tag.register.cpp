#include "rttr/registration.h"
#include "game/tag.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Tag>("Tag")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("value", &Tag::value);
}

