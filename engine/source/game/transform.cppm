module;

#include "preparser.h"
#include <string>

export module zeytin.game.transform;
import zeytin.component;

export struct Transform : public Component 
{
    Transform() = default; // Required default constructor
	//
	#if 0

    PROPERTY(READONLY, TRACK_VALUE)
    float positionX;

    PROPERTY(READONLY, TRACK_VALUE)
    float positionY;

    PROPERTY(READONLY, TRACK_VALUE)
    float positionZ;

    PROPERTY(READONLY)
    float rotationPitch;

    PROPERTY(READONLY)
    float rotationYaw;

    PROPERTY(READONLY)
    float rotationRoll;

    PROPERTY(READONLY)
    float scaleX;

    PROPERTY(READONLY)
    float scaleY;

    PROPERTY(READONLY)
    float scaleZ;

    PROPERTY()
    bool visible;

    PROPERTY(READONLY, ENABLE_IF=IsDebug())
    bool dirty;

    PROPERTY(READONLY)
    int layer;

    PROPERTY(READONLY)
    int entityId;

    PROPERTY(READONLY)
    std::string name;
	#endif
};

