module;

#include "preparser.h"

export module zeytin.game.scale;
import zeytin.component;

export struct Scale : public Component
{
	Scale() = default; // Should have a default constructor

	PROPERTY()
	float x = 1;

	PROPERTY()
	float y = 1;
};
