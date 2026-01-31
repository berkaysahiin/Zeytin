module;

#include "preparser.h"

export module zeytin.game.card_config;
import zeytin.component;

export struct GCardConfig final: public DataComponent
{
	PROPERTY(GROUP = "Layout")
	float card_width = 96.0F;

	PROPERTY(GROUP = "Layout")
	float card_height = 128.0F;

	PROPERTY(GROUP = "Timing")
	float flip_duration = 0.2F;

	PROPERTY(GROUP = "Timing")
	float mismatch_delay = 0.6F;

	PROPERTY(GROUP = "Card Back")
	float back_leather_tone = 0.45F;

	PROPERTY(GROUP = "Card Back")
	float back_texture_strength = 0.65F;

	PROPERTY(GROUP = "Card Back")
	float back_highlight_strength = 0.35F;

	PROPERTY(GROUP = "Card Back")
	float back_shadow_strength = 0.55F;

	PROPERTY(GROUP = "Card Back")
	float mask_scale = 0.48F;

	PROPERTY(GROUP = "Card Back")
	float mask_opacity = 0.9F;

	PROPERTY(GROUP = "Card Back")
	int mask_type = 0;
};
