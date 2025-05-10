#pragma once

#include "variant/variant_base.h"
#include "game/position.h"
#include "game/speed.h"
#include "game/player_info.h"
#include "game/collider.h"

#include <unordered_map>

enum class MappedKey {
    None,
    MoveUp,
    MoveDown,
    MoveRight,
    MoveLeft,
    CommonAbility,
    RandomAbility,
    Length,
};

class CharacterController : public VariantBase {
    VARIANT(CharacterController);
    REQUIRES(Position, Speed, PlayerInfo)

public:
    float acceleration = 5.0f; PROPERTY()
    float friction = 10.0f; PROPERTY()

    virtual void on_play_start() override;
    virtual void on_play_update() override;

private:
    void map_key_bindings();
    int get_keycode(const MappedKey key) const;

    void handle_input();
    void apply_movement();

    void bounce_from_boundries(Collider& other);
    void push_each_other(Collider& other);

private:
    Vector2 m_velocity = {0.0f, 0.0f};
    Vector2 m_input_direction = {0.0f, 0.0f};

    std::unordered_map<MappedKey, int> mapped_to_keycode;
};
