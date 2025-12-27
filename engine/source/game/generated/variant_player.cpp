#include "game/player.h"
#include "rttr/registration.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<Player>("Player")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("body_size", &Player::body_size)
        .property("enable_double_jump", &Player::enable_double_jump)
        .property("eye_offset_x", &Player::eye_offset_x)
        .property("eye_offset_y", &Player::eye_offset_y)
        .property("eye_size", &Player::eye_size)
        .property("eye_spacing", &Player::eye_spacing)
        .property("gravity", &Player::gravity)
        .property("hat_base_width", &Player::hat_base_width)
        .property("hat_color", &Player::hat_color)
        .property("hat_height", &Player::hat_height)
        .property("hat_offset_y", &Player::hat_offset_y)
        .property("hat_pom_color", &Player::hat_pom_color)
        .property("hat_pom_radius", &Player::hat_pom_radius)
        .property("hat_trim_color", &Player::hat_trim_color)
        .property("hat_trim_height", &Player::hat_trim_height)
        .property("jump_force", &Player::jump_force)
        .property("max_fall_speed", &Player::max_fall_speed)
        .property("max_jumps", &Player::max_jumps)
        .property("move_speed", &Player::move_speed)
        .property("squash_amount", &Player::squash_amount)
        .property("squash_speed", &Player::squash_speed)
        .property("stretch_amount", &Player::stretch_amount);
}
