module;

#include "preparser.h"
#include <string>

export module zeytin.game.position;
import zeytin.component;

export struct Position : public Component 
{
	PROPERTY() 
	float x = 31;

	PROPERTY() 
	float y = 32;

	PROPERTY(READONLY) 
	std::string name = "byebye";
};
