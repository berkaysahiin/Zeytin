module;

#include "preparser.h"

export module zeytin.game.scale;
import zeytin.component;

export struct Scale : public Component 
{
	Scale() = default; // Should have a default constructor

	PROPERTY(READONLY, ENABLE_IF=IsDebug(), TRACK_VALUE) 
	float x;

	PROPERTY(READONLY) 
	float y;
};
