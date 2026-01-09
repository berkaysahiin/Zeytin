module;

#include "preparser.h"
#include <cstdint>

export module zeytin.game.transform;
import zeytin.component;
import zeytin.entity;

export struct CTransform : public Component
{
    CTransform() = default;

    void on_update() override;

    PROPERTY(GROUP="Position")
    float position_x = 0.0f;
    PROPERTY(GROUP="Position")
    float position_y = 0.0f;

    PROPERTY(GROUP="Rotation")
    float rotation = 0.0f;

    PROPERTY(GROUP="Scale")
    float scale_x = 1.0f;
    PROPERTY(GROUP="Scale")
    float scale_y = 1.0f;

    // Note: not exposed via PROPERTY to avoid serialization issues, for now...
    uint64_t parent_id = 0;
};

