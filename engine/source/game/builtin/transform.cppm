module;

#include "preparser.h"

export module zeytin.game.transform;
import zeytin.component;
import zeytin.entity;

export struct CTransform : public DataComponent
{
    PROPERTY(GROUP="Position")
    float position_x = 0.0F;

    PROPERTY(GROUP="Position")
    float position_y = 0.0F;

    PROPERTY(GROUP="Rotation")
    float rotation = 0.0F;

    PROPERTY(GROUP="Scale")
    float scale_x = 1.0F;

    PROPERTY(GROUP="Scale")
    float scale_y = 1.0F;
};

