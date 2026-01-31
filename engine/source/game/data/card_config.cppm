module;

#include "preparser.h"

export module zeytin.game.card_config;
import zeytin.component;

export struct GCardConfig final: public DataComponent
{
	PROPERTY(GROUP = "Timing")
	float flip_duration = 0.2F;

	PROPERTY(GROUP = "Timing")
	float mismatch_delay = 0.6F;

	PROPERTY(GROUP = "Card Back")
	int mask_type = 0;
};
