#include "rttr/registration.h"

import zeytin.variant;

RTTR_REGISTRATION
{
    rttr::registration::class_<VariantCreateInfo>("VariantCreateInfo")
        .constructor<>()(rttr::policy::ctor::as_object);

    rttr::registration::class_<VariantBase>("VariantBase")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);
}
