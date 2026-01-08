module;

#include "preparser.h"

export module zeytin.game.position;
import zeytin.component;

#define component export struct
#define BODY_BEGIN : public Component {
#define BODY_END };

export struct Position : public Component 
{
	PROPERTY() 
	float x = 31;

	PROPERTY() 
	float y = 32;

	PROPERTY(SAVE)
	float _x;

	PROPERTY(HIDDEN)
	float _y;
};
