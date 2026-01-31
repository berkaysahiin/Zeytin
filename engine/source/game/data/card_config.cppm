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

	PROPERTY(GROUP = "Timing")
	float initial_reveal_duration = 2.0F;

	PROPERTY(GROUP = "Timing")
	float reveal_hold_after_timer = 0.5F;

	PROPERTY(GROUP = "Card Back")
	int mask_type = 0;
};
