#pragma once
#include "game/background.h"
#include "game/bomb.h"
#include "game/bullet.h"
#include "game/bullet_config.h"
#include "game/collider.h"
#include "game/countdown.h"
#include "game/crt_effect.h"
#include "game/diffuser.h"
#include "game/end_game.h"
#include "game/enemy.h"
#include "game/enemy_config.h"
#include "game/game_manager.h"
#include "game/objective_arrow.h"
#include "game/obstacle.h"
#include "game/player.h"
#include "game/position.h"
#include "game/rewind_effect.h"
#include "game/scale.h"
#include "game/time_controller.h"
#include "game/time_stopped_indicator.h"
#include "raylib.h"
#include "rttr/registration.h"
#include "variant/variant_base.h"

RTTR_REGISTRATION
{
    rttr::registration::class_<VariantCreateInfo>("VariantCreateInfo")
        .constructor<>()(rttr::policy::ctor::as_object);

    rttr::registration::class_<VariantBase>("VariantBase")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);

    rttr::registration::class_<Vector2>("Vector2")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("x", &Vector2::x)
        .property("y", &Vector2::y);

    rttr::registration::class_<Color>("Color")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("r", &Color::r)
        .property("g", &Color::g)
        .property("b", &Color::b)
        .property("a", &Color::a);

    rttr::registration::class_<Rectangle>("Rectangle")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("x", &Rectangle::x)
        .property("y", &Rectangle::y)
        .property("width", &Rectangle::width)
        .property("height", &Rectangle::height);

    rttr::registration::class_<Background>("Background")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("bg_color", &Background::bg_color)
        .property("star_count", &Background::star_count)
        .property("star_speed_max", &Background::star_speed_max)
        .property("star_speed_min", &Background::star_speed_min);

    rttr::registration::class_<Bomb>("Bomb")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("bar_height", &Bomb::bar_height)
        .property("bar_offset_y", &Bomb::bar_offset_y)
        .property("bar_width", &Bomb::bar_width)
        .property("bomb_size", &Bomb::bomb_size)
        .property("defuse_radius", &Bomb::defuse_radius)
        .property("defuse_time", &Bomb::defuse_time)
        .property("glow_size", &Bomb::glow_size)
        .property("label_size", &Bomb::label_size)
        .property("pulse_intensity", &Bomb::pulse_intensity)
        .property("pulse_speed", &Bomb::pulse_speed)
        .property("ring_speed", &Bomb::ring_speed)
        .property("shake_intensity", &Bomb::shake_intensity)
        .property("show_danger_rings", &Bomb::show_danger_rings);

    rttr::registration::class_<Bullet>("Bullet")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);

    rttr::registration::class_<BulletConfig>("BulletConfig")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("airtime_threshold", &BulletConfig::airtime_threshold)
        .property("height", &BulletConfig::height)
        .property("lifetime", &BulletConfig::lifetime)
        .property("speed", &BulletConfig::speed)
        .property("width", &BulletConfig::width);

    rttr::registration::class_<CRTEffect>("CRTEffect")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("chromatic_aberration", &CRTEffect::chromatic_aberration)
        .property("curvature", &CRTEffect::curvature)
        .property("enabled", &CRTEffect::enabled)
        .property("noise_intensity", &CRTEffect::noise_intensity)
        .property("scanline_intensity", &CRTEffect::scanline_intensity)
        .property("vignette_intensity", &CRTEffect::vignette_intensity);

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

    rttr::registration::class_<Diffuser>("Diffuser")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("bob_amount", &Diffuser::bob_amount)
        .property("bob_speed", &Diffuser::bob_speed)
        .property("icon_offset_y", &Diffuser::icon_offset_y)
        .property("icon_size", &Diffuser::icon_size)
        .property("pickup_radius", &Diffuser::pickup_radius);

    rttr::registration::class_<EndGame>("EndGame")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("game_over_font_size", &EndGame::game_over_font_size)
        .property("reason_font_size", &EndGame::reason_font_size)
        .property("ui_transparency", &EndGame::ui_transparency)
        .property("win_font_size", &EndGame::win_font_size);

    rttr::registration::class_<Enemy>("Enemy")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("body_size", &Enemy::body_size)
        .property("death_fade_duration", &Enemy::death_fade_duration)
        .property("eye_offset_x", &Enemy::eye_offset_x)
        .property("eye_offset_y", &Enemy::eye_offset_y)
        .property("eye_size", &Enemy::eye_size)
        .property("eye_spacing", &Enemy::eye_spacing)
        .property("gravity", &Enemy::gravity)
        .property("gun_length", &Enemy::gun_length)
        .property("gun_offset_x", &Enemy::gun_offset_x)
        .property("gun_offset_y", &Enemy::gun_offset_y)
        .property("gun_width", &Enemy::gun_width)
        .property("max_fall_speed", &Enemy::max_fall_speed)
        .property("patrol_distance", &Enemy::patrol_distance)
        .property("patrol_speed", &Enemy::patrol_speed)
        .property("shoot_interval", &Enemy::shoot_interval)
        .property("shoot_range", &Enemy::shoot_range)
        .property("use_global_config", &Enemy::use_global_config);

    rttr::registration::class_<EnemyConfig>("EnemyConfig")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("body_size", &EnemyConfig::body_size)
        .property("death_fade_duration", &EnemyConfig::death_fade_duration)
        .property("eye_offset_x", &EnemyConfig::eye_offset_x)
        .property("eye_offset_y", &EnemyConfig::eye_offset_y)
        .property("eye_size", &EnemyConfig::eye_size)
        .property("eye_spacing", &EnemyConfig::eye_spacing)
        .property("gravity", &EnemyConfig::gravity)
        .property("gun_length", &EnemyConfig::gun_length)
        .property("gun_offset_x", &EnemyConfig::gun_offset_x)
        .property("gun_offset_y", &EnemyConfig::gun_offset_y)
        .property("gun_width", &EnemyConfig::gun_width)
        .property("max_fall_speed", &EnemyConfig::max_fall_speed)
        .property("patrol_distance", &EnemyConfig::patrol_distance)
        .property("patrol_speed", &EnemyConfig::patrol_speed)
        .property("shoot_interval", &EnemyConfig::shoot_interval)
        .property("shoot_range", &EnemyConfig::shoot_range);

    rttr::registration::class_<GameManager>("GameManager")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object);

    rttr::registration::class_<ObjectiveArrow>("ObjectiveArrow")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("arrow_height", &ObjectiveArrow::arrow_height)
        .property("arrow_size", &ObjectiveArrow::arrow_size)
        .property("bounce_amount", &ObjectiveArrow::bounce_amount)
        .property("bounce_speed", &ObjectiveArrow::bounce_speed);

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
        .property("move_speed", &Player::move_speed)
        .property("squash_amount", &Player::squash_amount)
        .property("squash_speed", &Player::squash_speed)
        .property("stretch_amount", &Player::stretch_amount);

    rttr::registration::class_<Position>("Position")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("x", &Position::x)
        .property("y", &Position::y);

    rttr::registration::class_<RewindEffect>("RewindEffect")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("arrow_bg_color", &RewindEffect::arrow_bg_color)
        .property("arrow_color", &RewindEffect::arrow_color)
        .property("arrow_count", &RewindEffect::arrow_count)
        .property("arrow_size", &RewindEffect::arrow_size)
        .property("arrow_spacing", &RewindEffect::arrow_spacing)
        .property("enabled", &RewindEffect::enabled)
        .property("pulse_intensity", &RewindEffect::pulse_intensity)
        .property("pulse_speed", &RewindEffect::pulse_speed);

    rttr::registration::class_<Scale>("Scale")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("x", &Scale::x)
        .property("y", &Scale::y);

    rttr::registration::class_<Star>("Star")
        .constructor<>()(rttr::policy::ctor::as_object)
        .property("brightness", &Star::brightness)
        .property("size", &Star::size)
        .property("speed", &Star::speed)
        .property("x", &Star::x)
        .property("y", &Star::y)
        (rttr::metadata("NO_VARIANT", true));

    rttr::registration::class_<TimeController>("TimeController")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("max_history_frames", &TimeController::max_history_frames);

    rttr::registration::class_<TimeStoppedIndicator>("TimeStoppedIndicator")
        .constructor<>()(rttr::policy::ctor::as_object)
        .constructor<VariantCreateInfo>()(rttr::policy::ctor::as_object)
        .property("activate_threshhold", &TimeStoppedIndicator::activate_threshhold)
        .property("enabled", &TimeStoppedIndicator::enabled)
        .property("font_size", &TimeStoppedIndicator::font_size)
        .property("padding", &TimeStoppedIndicator::padding);

}
