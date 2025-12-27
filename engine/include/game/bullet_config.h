#pragma once

#include "variant/variant_base.h"
#include "raylib.h"

class BulletConfig : public VariantBase {
    VARIANT(BulletConfig);
	DATA_CLASS()

    float speed = 400.0f; PROPERTY();
    float lifetime = 3.0f; PROPERTY();
    float width = 8.0f; PROPERTY();
    float height = 4.0f; PROPERTY();
    
    Color bullet_color = {255, 200, 0, 255}; // Yellow/orange
};
