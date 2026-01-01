module;

#include "preparser.h"
#include <string>

export module zeytin.game.position;
import zeytin.component;

export struct Position : public Component 
{
	Position() = default; // Should have a default constructor

	PROPERTY(READONLY, ENABLE_IF=IsDebug(), TRACK_VALUE) 
	float x;

	PROPERTY(READONLY) 
	float y;

	PROPERTY(READONLY) 
	std::string name;

	bool IsDebug();
};
