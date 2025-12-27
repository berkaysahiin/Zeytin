#include "game/time_controller.h"
#include "core/query.h"
#include "core/raylib_wrapper.h"
#include "game/player.h"
#include "game/enemy.h"
#include "game/bullet.h"
#include "game/countdown.h"
#include "game/position.h"
#include "game/scale.h"
#include "game/collider.h"

void TimeController::on_init() {
	m_history.resize(max_history_frames);
}

void TimeController::on_play_update() {
    m_is_rewinding = should_rewind();
    
    if (m_is_rewinding) {
        apply_rewind();
    } else {
        record_snapshot();
    }
}

bool TimeController::should_rewind() {
    auto player_opt = Query::try_find_first<Player>();
    if (!player_opt) return false;
    
    bool pressing_left = is_key_down(KEY_A) || is_key_down(KEY_LEFT);
    bool pressing_right = is_key_down(KEY_D) || is_key_down(KEY_RIGHT);
    
    return pressing_left && !pressing_right;
}

void TimeController::record_snapshot() {
    GameSnapshot snapshot;
    
    auto countdown_opt = Query::try_find_first<Countdown>();
    if (countdown_opt) {
        snapshot.countdown_time = countdown_opt->get().get_time_remaining();
    }
    
    auto enemy_entities = Query::find_all_with<Enemy, Position>();
    for (::entity_id id : enemy_entities) {
        auto& enemy = Query::get<Enemy>(id);
        auto& pos = Query::get<Position>(id);
        
        GameSnapshot::EnemyState state;
        state.id = id;
        state.position = {pos.x, pos.y};
        state.velocity = enemy.get_velocity();
        state.is_grounded = enemy.get_is_grounded();
        state.patrol_direction = enemy.get_patrol_direction();
        state.shoot_timer = enemy.get_shoot_timer();
        state.is_dead = enemy.is_dead();
        
        snapshot.enemies.push_back(state);
    }
    
    auto bullet_entities = Query::find_all_with<Bullet, Position>();
    for (::entity_id id : bullet_entities) {
        auto& bullet = Query::get<Bullet>(id);
        
        if (bullet.is_dead) continue;
        
        auto& pos = Query::get<Position>(id);
        
        GameSnapshot::BulletState state;
        state.position = {pos.x, pos.y};
        state.time_alive = bullet.get_time_alive();
        state.direction = bullet.get_direction();
        
        snapshot.bullets.push_back(state);
    }
    
    m_history.push_back(snapshot);
    
    if (m_history.size() > static_cast<size_t>(max_history_frames)) {
        m_history.pop_front();
    }
}

void TimeController::apply_rewind() {
    if (m_history.empty()) return;
    
    GameSnapshot snapshot = m_history.back();
    m_history.pop_back();
    
    for (const auto& enemy_state : snapshot.enemies) {
        if (!Query::entity_exists(enemy_state.id)) continue;
        
        auto enemy_opt = Query::try_get<Enemy>(enemy_state.id);
        auto pos_opt = Query::try_get<Position>(enemy_state.id);
        
        if (enemy_opt && pos_opt) {
            auto& pos = pos_opt->get();
            pos.x = enemy_state.position.x;
            pos.y = enemy_state.position.y;
            
            auto& enemy = enemy_opt->get();
            enemy.set_velocity(enemy_state.velocity);
            enemy.set_is_grounded(enemy_state.is_grounded);
            enemy.set_patrol_direction(enemy_state.patrol_direction);
            enemy.set_shoot_timer(enemy_state.shoot_timer);
            enemy.set_is_dead(enemy_state.is_dead);
        }
    }
    
    auto current_bullets = Query::find_all_with<Bullet>();
    for (::entity_id bullet_id : current_bullets) {
        Query::remove_entity(bullet_id);
    }
    
    for (const auto& bullet_state : snapshot.bullets) {
        ::entity_id bullet_id = Query::create_entity();
        
        auto pos_opt = Query::add<Position>(bullet_id);
        if (pos_opt) {
            auto& pos = pos_opt->get();
            pos.x = bullet_state.position.x;
            pos.y = bullet_state.position.y;
        }
        
        Query::add<Scale>(bullet_id);
        Query::add<Collider>(bullet_id);
        
        auto bullet_opt = Query::add<Bullet>(bullet_id);
        if (bullet_opt) {
            auto& bullet = bullet_opt->get();
            bullet.set_direction(bullet_state.direction);
            bullet.set_time_alive(bullet_state.time_alive);
        }
    }
    
    auto countdown_opt = Query::try_find_first<Countdown>();
    if (countdown_opt) {
        auto& countdown = countdown_opt->get();
        countdown.set_time_remaining(snapshot.countdown_time);
    }
}
