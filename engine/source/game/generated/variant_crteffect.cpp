#include "game/crt_effect.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<CRTEffect>("CRTEffect")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("chromatic_aberration", &CRTEffect::chromatic_aberration)
        .property("curvature", &CRTEffect::curvature)
        .property("enabled", &CRTEffect::enabled)
        .property("noise_intensity", &CRTEffect::noise_intensity)
        .property("scanline_intensity", &CRTEffect::scanline_intensity)
        .property("vignette_intensity", &CRTEffect::vignette_intensity);
}
