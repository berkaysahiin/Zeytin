#include "game/time_controller.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<TimeController>("TimeController")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("max_history_frames", &TimeController::max_history_frames);
}
