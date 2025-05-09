#pragma once
#include "game/character_controller.h"
#include "game/collider.h"
#include "game/player_info.h"
#include "game/position.h"
#include "game/scale.h"
#include "game/speed.h"
#include "game/sprite.h"
#include "game/tag.h"
#include "game/velocity.h"
#include "game/zone.h"
#include "raylib.h"
#include "rttr/registration.h"

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

    rttr::registration::class_<CharacterController>("CharacterController")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("acceleration", &CharacterController::acceleration)
        .property("friction", &CharacterController::friction);

    rttr::registration::class_<Collider>("Collider")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("m_collider_type", &Collider::m_collider_type)
        .property("m_draw_debug", &Collider::m_draw_debug)
        .property("m_height", &Collider::m_height)
        .property("m_is_trigger", &Collider::m_is_trigger)
        .property("m_radius", &Collider::m_radius)
        .property("m_static", &Collider::m_static)
        .property("m_width", &Collider::m_width)
        .property("thickness", &Collider::thickness);

    rttr::registration::class_<PlayerInfo>("PlayerInfo")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("color", &PlayerInfo::color)
        .property("index", &PlayerInfo::index)
        .property("name", &PlayerInfo::name);

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

    rttr::registration::class_<Speed>("Speed")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("value", &Speed::value);

    rttr::registration::class_<Sprite>("Sprite")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("path_to_sprite", &Sprite::path_to_sprite)(rttr::metadata("SET_CALLBACK", "handle_new_path"))

        .method("handle_new_path", &Sprite::handle_new_path);

    rttr::registration::class_<Tag>("Tag")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("value", &Tag::value);

    rttr::registration::class_<Velocity>("Velocity")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("x", &Velocity::x)
        .property("y", &Velocity::y);

    rttr::registration::class_<Zone>("Zone")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);

}
