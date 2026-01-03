module;

#include "preparser.h"

export module zeytin.game.position;
import zeytin.component;

export struct Position : public Component 
{
	Position() = default; // Should have a default constructor

	PROPERTY() 
	float x = 31;

	PROPERTY() 
	float y = 32;
};
