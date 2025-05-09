#pragma once

#include "variant/variant_base.h"
#include "game/position.h"
#include "game/scale.h"

class PlayerDrawer : public VariantBase {
    VARIANT(PlayerDrawer);
    REQUIRES(Position, Scale)

public:
    float width = 50.0f; PROPERTY()
    float height = 50.0f; PROPERTY()
    
    void on_update() override;
};
