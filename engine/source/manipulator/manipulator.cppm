module;

#include <cstdint>
#include <string_view>
#include "raylib.h"

export module zeytin.manipulator;
import zeytin.game.transform;

using EntityID = uint64_t;

export enum class ManipulatorType : uint8_t {
    Translate,
    Rotate,
    Scale,
};

export struct Context {
    CTransform& transform;
    const EntityID entity_id;
    const Camera2D& camera;
    const bool is_play_mode;
};

export class IManipulator {
public:
    virtual ~IManipulator() = default;

    virtual void update(Context& ctx) = 0;
    virtual bool is_active() const = 0;
    virtual void reset() = 0;
    virtual std::string_view get_name() const = 0;
};
