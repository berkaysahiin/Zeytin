module;

#include "preparser.h"
#include <string>

export module zeytin.game.base_stats;
import zeytin.component;

export struct CBaseStats : Component
{
	VIRTUAL_COMPONENT

    PROPERTY(NAME="Health", MIN=0, MAX=200, TOOLTIP="Base health value")
    int health = 100;

    PROPERTY(NAME="Mana", TOOLTIP="Mana pool")
    float mana = 50.0f;

    PROPERTY(NAME="Icon Path", FILEPATH, TOOLTIP="Icon path")
    std::string icon_path = "";
};
