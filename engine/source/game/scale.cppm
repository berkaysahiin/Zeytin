module;

#include "preparser.h"
#include <string>

export module zeytin.game.scale;
import zeytin.component;

export struct Vector2Component : public Component
{
	PROPERTY(READONLY, ENABLE_IF=IsDebug(), TRACK_VALUE) 
	float x = 1;

	PROPERTY(READONLY) 
	float y = 1;

	float otuzbir = 31;
};

export struct Scale : public Vector2Component 
{
	Scale() = default; // Should have a default constructor

	PROPERTY()
	std::string debug_name = "Scale";

	PROPERTY()
	int test = 2;

	PROPERTY()
	bool debug = false;
};
