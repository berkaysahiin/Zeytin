module;

#include "preparser.h"
#include <string>

export module zeytin.game.massive_component;
import zeytin.component;

export struct MassiveComponent : public Component 
{
    MassiveComponent() = default;

    PROPERTY(READONLY)
    int id;

    PROPERTY(READONLY)
    std::string name;

    PROPERTY(READONLY)
    std::string tag;

    PROPERTY(READONLY, TRACK_VALUE)
    float posX;

    PROPERTY(READONLY, TRACK_VALUE)
    float posY;

    PROPERTY(READONLY, TRACK_VALUE)
    float posZ;

    PROPERTY(READONLY)
    float rotX;

    PROPERTY(READONLY)
    float rotY;

    PROPERTY(READONLY)
    float rotZ;

    PROPERTY(READONLY)
    float scaleX;

    PROPERTY(READONLY)
    float scaleY;

    PROPERTY(READONLY)
    float scaleZ;

    PROPERTY()
    bool dynamic;

    PROPERTY(READONLY)
    float mass;

    PROPERTY(READONLY)
    float friction;

    PROPERTY(READONLY)
    float restitution;

    PROPERTY(READONLY)
    bool gravityEnabled;

    PROPERTY(READONLY)
    bool visible;

    PROPERTY(READONLY)
    int renderLayer;

    PROPERTY(READONLY)
    float opacity;

    PROPERTY(READONLY)
    bool animated;

    PROPERTY(READONLY)
    float animationTime;

    PROPERTY(READONLY)
    float animationSpeed;

    PROPERTY(READONLY)
    bool active;

    PROPERTY(READONLY)
    int state;

    PROPERTY(READONLY)
    float awarenessRadius;

    PROPERTY(READONLY)
    float aggression;

    PROPERTY(READONLY)
    int networkId;

    PROPERTY(READONLY)
    bool replicated;

    PROPERTY(READONLY)
    float lastSyncTime;

    PROPERTY(READONLY, ENABLE_IF=IsDebug())
    bool debugSelected;

    PROPERTY(READONLY, ENABLE_IF=IsDebug())
    float debugValue;

    PROPERTY(READONLY, ENABLE_IF=IsDebug())
    std::string debugLabel;
};

