module;

#include "preparser.h"

export module zeytin.game.background;
import zeytin.component;

export struct CBackground final : public Component
{
	PROPERTY(GROUP="Background")
	bool enabled = true;

	PROPERTY(GROUP="Background")
	int base_red = 24;

	PROPERTY(GROUP="Background")
	int base_green = 18;

	PROPERTY(GROUP="Background")
	int base_blue = 14;

	PROPERTY(GROUP="Background")
	int base_alpha = 255;

	PROPERTY(GROUP="Background")
	int accent_red = 48;

	PROPERTY(GROUP="Background")
	int accent_green = 32;

	PROPERTY(GROUP="Background")
	int accent_blue = 24;

	PROPERTY(GROUP="Background")
	int accent_alpha = 120;

	PROPERTY(GROUP="Background")
	float motion_speed = 0.2F;

	PROPERTY(GROUP="Background")
	float motion_strength = 0.08F;

	PROPERTY(GROUP="Background")
	float blob_scale = 0.75F;

	void on_early_update() override;
};
