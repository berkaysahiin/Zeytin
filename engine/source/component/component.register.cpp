#include "rttr/registration.h"

import zeytin.component;

RTTR_REGISTRATION
{
    rttr::registration::class_<Component>("Component")
        .constructor<>()(rttr::policy::ctor::as_object);

    rttr::registration::class_<DataComponent>("DataComponent")
        .constructor<>()(rttr::policy::ctor::as_object);
}
