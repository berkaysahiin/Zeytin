module;

#include "preparser.h"

export module zeytin.game.intermediate_stats;
import zeytin.component;

export struct CIntermediateStats : Component
{
    PROPERTY(READONLY_IN_DERIVED, NAME="Armor", MIN=0, MAX=100, TOOLTIP="Armor from gear")
    int armor = 15;

    PROPERTY(READONLY_IN_DERIVED, NAME="Debug Boost", TOOLTIP="Hidden boost for testing")
    float debug_boost2 = 4.25f;

    PROPERTY(READONLY_IN_DERIVED, NAME="Debug Boost", TOOLTIP="Hidden boost for testing")
    float debug_boost3 = 4.25f;
};

