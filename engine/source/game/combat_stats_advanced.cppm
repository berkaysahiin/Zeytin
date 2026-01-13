module;

#include "preparser.h"

export module zeytin.game.combat_stats_advanced;
import zeytin.component;
import zeytin.game.combat_stats_base;

export struct CCombatStatsAdvanced : CCombatStatsBase
{
	VIRTUAL_COMPONENT

    PROPERTY(NAME="Crit Chance", MIN=0, MAX=100, TOOLTIP="Critical hit chance")
    float crit_chance = 5.0f;

    PROPERTY(NAME="Crit Multiplier", MIN=1, MAX=5, TOOLTIP="Critical hit multiplier")
    float crit_multiplier = 1.5f;
};
