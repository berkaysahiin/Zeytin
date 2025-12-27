// engine/include/game/test_variant.h
#pragma once

#include "variant/variant_base.h"

class TestVariant : public VariantBase {
    VARIANT(TestVariant);

public:
    // Integer properties
    int health = 100; PROPERTY();
    int max_health = 100; PROPERTY();
    int level = 1; PROPERTY();
    
    // Float properties
    float speed = 5.0f; PROPERTY();
    float jump_force = 10.0f; PROPERTY();
    float gravity = 0.5f; PROPERTY();
    
    // Boolean properties
    bool is_active = true; PROPERTY();
    bool can_move = true; PROPERTY();
    bool is_invincible = false; PROPERTY();
    
    // String properties
    std::string name = "Player"; PROPERTY();
    std::string tag = ""; PROPERTY();
};
