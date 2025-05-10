#pragma once

#include "variant/variant_base.h"

class PlayerInfo : public VariantBase {
    VARIANT(PlayerInfo);

public:
    int index = 0; PROPERTY();
    std::string name; PROPERTY();
    Color color = BLUE; PROPERTY();

    float time_spent_zone = 0; PROPERTY();
};


