#pragma once
#include "game/character_controller.h"
#include "game/collider.h"
#include "game/particle_system.h"
#include "game/player_info.h"
#include "game/player_renderer.h"
#include "game/position.h"
#include "game/powerup.h"
#include "game/powerup_spawner.h"
#include "game/scale.h"
#include "game/speed.h"
#include "game/sprite.h"
#include "game/tag.h"
#include "game/velocity.h"
#include "game/wall.h"
#include "game/zone.h"
#include "game/zone_manager.h"
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

    rttr::registration::class_<Particle>("Particle")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("color", &Particle::color)
        .property("lifetime", &Particle::lifetime)
        .property("max_lifetime", &Particle::max_lifetime)
        .property("position", &Particle::position)
        .property("velocity", &Particle::velocity)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<ParticleSystem>("ParticleSystem")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("MAX_PARTICLES", &ParticleSystem::MAX_PARTICLES)
        .property("PARTICLE_LIFETIME", &ParticleSystem::PARTICLE_LIFETIME)
        .property("PARTICLE_SPEED", &ParticleSystem::PARTICLE_SPEED);

    rttr::registration::class_<PlayerInfo>("PlayerInfo")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("color", &PlayerInfo::color)
        .property("in_zone", &PlayerInfo::in_zone)
        .property("index", &PlayerInfo::index)
        .property("m_padding", &PlayerInfo::m_padding)
        .property("m_panel_height", &PlayerInfo::m_panel_height)
        .property("m_panel_width", &PlayerInfo::m_panel_width)
        .property("m_panel_x", &PlayerInfo::m_panel_x)
        .property("m_panel_y", &PlayerInfo::m_panel_y)
        .property("name", &PlayerInfo::name)
        .property("shield", &PlayerInfo::shield)
        .property("time_spent_zone", &PlayerInfo::time_spent_zone);

    rttr::registration::class_<PlayerRenderer>("PlayerRenderer")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("corner_radius", &PlayerRenderer::corner_radius)
        .property("corner_segments", &PlayerRenderer::corner_segments)
        .property("m_max_history_length", &PlayerRenderer::m_max_history_length)
        .property("m_trail_fade_factor", &PlayerRenderer::m_trail_fade_factor)
        .property("outline_thickness", &PlayerRenderer::outline_thickness)
        .property("player_label_size", &PlayerRenderer::player_label_size)
        .property("pulse_intensity", &PlayerRenderer::pulse_intensity)
        .property("pulse_speed", &PlayerRenderer::pulse_speed)
        .property("show_player_number", &PlayerRenderer::show_player_number)
        .property("show_trail", &PlayerRenderer::show_trail)
        .property("trail_thickness", &PlayerRenderer::trail_thickness)
        .property("use_outline", &PlayerRenderer::use_outline);

    rttr::registration::class_<Position>("Position")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("x", &Position::x)
        .property("y", &Position::y);

    rttr::registration::class_<PowerUp>("PowerUp")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);

    rttr::registration::class_<PowerUpSpawner>("PowerUpSpawner")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("m_max_lifetime", &PowerUpSpawner::m_max_lifetime)
        .property("m_min_lifetime", &PowerUpSpawner::m_min_lifetime)
        .property("m_randomize_type", &PowerUpSpawner::m_randomize_type)
        .property("m_show_debug_visuals", &PowerUpSpawner::m_show_debug_visuals)
        .property("m_spawn_interval", &PowerUpSpawner::m_spawn_interval)
        .property("m_spawn_radius", &PowerUpSpawner::m_spawn_radius);

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

    rttr::registration::class_<Wall>("Wall")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);

    rttr::registration::class_<Zone>("Zone")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);

    rttr::registration::class_<ZoneManager>("ZoneManager")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("m_max_radius", &ZoneManager::m_max_radius)
        .property("m_max_secs", &ZoneManager::m_max_secs)
        .property("m_min_radius", &ZoneManager::m_min_radius)
        .property("m_min_secs", &ZoneManager::m_min_secs)
        .property("m_spawn_every_secs", &ZoneManager::m_spawn_every_secs)
        .property("m_thick_incess_rate", &ZoneManager::m_thick_incess_rate)
        .property("show_debug_visuals", &ZoneManager::show_debug_visuals);

}
