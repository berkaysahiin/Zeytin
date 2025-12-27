// PP_SKIP

#pragma once

#include "entity/entity.h"
#include "raylib.h"
#include <vector>

struct GameSnapshot {
    struct EnemyState {
        entity_id id;
        Vector2 position;
        Vector2 velocity;
        bool is_grounded;
        int patrol_direction;
        float shoot_timer;
        bool is_dead;
    };
    
    struct BulletState {
        Vector2 position;
        float time_alive;
        int direction;
    };
    
    std::vector<EnemyState> enemies;
    std::vector<BulletState> bullets;
    float countdown_time;
	float bomb_defuse_progress;
};
