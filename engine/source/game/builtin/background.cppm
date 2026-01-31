module;

#include "preparser.h"
#include <string>

export module zeytin.game.background;
import zeytin.component;

export struct CBackground final : public Component
{
	PROPERTY(GROUP="Background")
	bool enabled = true;

	PROPERTY(GROUP="Background")
	std::string texture_path = "background.png";

	void on_early_update() override;
};
