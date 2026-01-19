module;

#include "preparser.h"
#include <string>

export module zeytin.game.massive_component;
import zeytin.component;

export struct MassiveComponent : public Component 
{
    MassiveComponent() = default;

    PROPERTY()
    int id;
};

