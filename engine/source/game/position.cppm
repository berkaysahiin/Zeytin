module;

#include "preparser.h"
#include <string>

export module zeytin.game.position;
import zeytin.component;

export struct Position : public Component 
{
	Position() = default; // Should have a default constructor

	PROPERTY() 
	float x = 31;

	PROPERTY() 
	float y = 32;

	PROPERTY() 
	std::string name;

	bool IsDebug();
};
