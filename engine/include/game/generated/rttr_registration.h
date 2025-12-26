#pragma once
#include "game/collider.h"
#include "game/countdown.h"
#include "game/obstacle.h"
#include "game/player.h"
#include "game/position.h"
#include "game/scale.h"
#include "raylib.h"
#include "rttr/registration.h"
#include "variant/variant_base.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<VariantCreateInfo>("VariantCreateInfo")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("entity_id", &VariantCreateInfo::entity_id);
    
    rttr::registration::class_<VariantBase>("VariantBase")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("entity_id", &VariantBase::entity_id)(rttr::metadata("NO_SERIALIZE", true));

    rttr::registration::class_<Vector2>("Vector2")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("x", &Vector2::x)
        .property("y", &Vector2::y)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<Vector3>("Vector3")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("x", &Vector3::x)
        .property("y", &Vector3::y)
        .property("z", &Vector3::z)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<Rectangle>("Rectangle")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("x", &Rectangle::x)
        .property("y", &Rectangle::y)
        .property("width", &Rectangle::width)
        .property("height", &Rectangle::height)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<Color>("Color")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("r", &Color::r)
        .property("g", &Color::g)
        .property("b", &Color::b)
        .property("a", &Color::a)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<Camera2D>("Camera2D")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("offset", &Camera2D::offset)
        .property("target", &Camera2D::target)
        .property("rotation", &Camera2D::rotation)
        .property("zoom", &Camera2D::zoom)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<Texture2D>("Texture2D")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("id", &Texture2D::id)
        .property("width", &Texture2D::width)
        .property("height", &Texture2D::height)
        .property("mipmaps", &Texture2D::mipmaps)
        .property("format", &Texture2D::format)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<Collider>("Collider")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("height", &Collider::height)
        .property("show_debug", &Collider::show_debug)
        .property("width", &Collider::width);

    rttr::registration::class_<Countdown>("Countdown")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("countdown_active", &Countdown::countdown_active)
        .property("danger_threshold", &Countdown::danger_threshold)
        .property("duration", &Countdown::duration)
        .property("font_size", &Countdown::font_size)
        .property("offset_x", &Countdown::offset_x)
        .property("offset_y", &Countdown::offset_y)
        .property("pause_when_zero", &Countdown::pause_when_zero)
        .property("warning_threshold", &Countdown::warning_threshold);

    rttr::registration::class_<Obstacle>("Obstacle")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("outline_thickness", &Obstacle::outline_thickness)
        .property("show_outline", &Obstacle::show_outline);

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
        .property("jump_force", &Player::jump_force)
        .property("max_fall_speed", &Player::max_fall_speed)
        .property("max_jumps", &Player::max_jumps)
        .property("move_speed", &Player::move_speed);

    rttr::registration::class_<Position>("Position")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("x", &Position::x)
        .property("y", &Position::y);

    rttr::registration::class_<Scale>("Scale")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("x", &Scale::x)
        .property("y", &Scale::y);

}
