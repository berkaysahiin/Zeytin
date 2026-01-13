module;

#include "preparser.h"
#include <string>

export module zeytin.game.combat_stats;
import zeytin.component;
import zeytin.game.combat_stats_advanced;

export struct CCombatStats : CCombatStatsAdvanced
{
    PROPERTY(NAME="Weapon Name", TOOLTIP="Equipped weapon name")
    std::string weapon_name = "";

    PROPERTY(NAME="Armor Rating", MIN=0, MAX=500, TOOLTIP="Armor rating")
    int armor_rating = 0;
};
