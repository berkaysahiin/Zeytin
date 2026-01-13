module;

#include "preparser.h"

export module zeytin.game.combat_stats_base;
import zeytin.component;

export struct CCombatStatsBase : Component
{
	VIRTUAL_COMPONENT

    PROPERTY(NAME="Strength", MIN=0, MAX=200, TOOLTIP="Base strength")
    int strength = 10;

    PROPERTY(NAME="Agility", MIN=0, MAX=200, TOOLTIP="Base agility")
    int agility = 10;
};
