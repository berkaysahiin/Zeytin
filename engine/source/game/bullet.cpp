#include "game/bullet.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/game_manager.h"
#include "game/position.h"
#include "game/scale.h"
#include "game/collider.h"
#include "game/bullet_config.h"

void Bullet::on_init() {
    m_time_alive = 0.0f;
    m_config_loaded = false;
    
    auto config_opt = Query::try_find_first<BulletConfig>();
    if (config_opt) {
        auto& config = config_opt->get();
        m_speed = config.speed;
        m_lifetime = config.lifetime;
        m_width = config.width;
        m_height = config.height;
        m_bullet_color = config.bullet_color;
        m_airtime_threshold = config.airtime_threshold;
        m_config_loaded = true;
    }
    
    auto& collider = Query::get<Collider>(this);
    collider.width = m_width;
    collider.height = m_height;
}

void Bullet::on_update() {
    draw_bullet();
}

void Bullet::on_play_update() {
	auto game_mgr_opt = Query::try_find_first<GameManager>();
    if (!game_mgr_opt || !game_mgr_opt->get().should_game_run()) {
        return;
    }

    move();
    check_lifetime();
}

void Bullet::move() {
    auto& position = Query::get<Position>(this);
    float delta = get_frame_time();
    
    position.x += m_direction * m_speed * delta;
}

void Bullet::check_lifetime() {
    float delta = get_frame_time();
    m_time_alive += delta;
    
    if (m_time_alive >= m_lifetime) {
        this->is_dead = true;
    }
}

void Bullet::draw_bullet() {
    auto [position, scale] = Query::read<Position, Scale>(this);
    
    float scaled_width = m_width * scale.x;
    float scaled_height = m_height * scale.y;
    
    Rectangle bullet_rect = {
        position.x - scaled_width / 2,
        position.y - scaled_height / 2,
        scaled_width,
        scaled_height
    };
    
    draw_rectangle_rec(bullet_rect, m_bullet_color);
}
