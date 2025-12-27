#include "game/game_manager.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<GameManager>("GameManager")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("pos_x", &GameManager::pos_x)
        .property("pos_y", &GameManager::pos_y)
        .property("size", &GameManager::size);
}
