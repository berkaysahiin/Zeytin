#include "rttr/registration.h"
#include "game/sprite.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Sprite>("Sprite")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("path_to_sprite", &Sprite::path_to_sprite)(rttr::metadata("SET_CALLBACK", "handle_new_path"))

        .method("handle_new_path", &Sprite::handle_new_path);
}

