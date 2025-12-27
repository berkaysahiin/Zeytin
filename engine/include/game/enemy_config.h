#pragma once

#include "variant/variant_base.h"

class EnemyConfig : public VariantBase {
    VARIANT(EnemyConfig);
    DATA_CLASS()

    float patrol_speed = 150.0f; PROPERTY();
    float patrol_distance = 200.0f; PROPERTY();
    
    float gravity = 1500.0f; PROPERTY();
    float max_fall_speed = 800.0f; PROPERTY();
    
    float shoot_interval = 2.0f; PROPERTY();
    float shoot_range = 500.0f; PROPERTY();
    
    float body_size = 50.0f; PROPERTY();
    float eye_size = 6.0f; PROPERTY();
    float eye_offset_x = 10.0f; PROPERTY();
    float eye_offset_y = -8.0f; PROPERTY();
    float eye_spacing = 8.0f; PROPERTY();
    
    float gun_length = 25.0f; PROPERTY();
    float gun_width = 8.0f; PROPERTY();
    float gun_offset_x = 15.0f; PROPERTY();
    float gun_offset_y = 5.0f; PROPERTY();

    float death_fade_duration = 1.0f; PROPERTY();
};
