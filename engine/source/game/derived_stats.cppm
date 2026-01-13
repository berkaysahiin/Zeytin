module;

#include "preparser.h"

export module zeytin.game.derived_stats;
import zeytin.component;
import zeytin.game.base_stats;

export struct CDerivedStats : CBaseStats
{
    PROPERTY(MIN=0, MAX=100, TOOLTIP="Stamina pool")
    float stamina = 25.0f;

    PROPERTY(MIN=0, MAX=20, TOOLTIP="Movement speed")
    float speed = 5.0f;
};
