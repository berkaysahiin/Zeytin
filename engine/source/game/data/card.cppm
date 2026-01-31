module;

#include "preparser.h"
#include <cstdint>

export module zeytin.game.card;
import zeytin.component;

export enum class CardMaskStatus : std::uint8_t {
    NO_MASK = 0,
    SMILE = 1,
    SAD = 2
};

export struct CCard final : public DataComponent
{
    PROPERTY(GROUP="Identity")
    int symbol_id = 0;

    CardMaskStatus e_mask_status = CardMaskStatus::NO_MASK;

    PROPERTY(GROUP="Grid")
    int row = 0;

    PROPERTY(GROUP="Grid")
    int column = 0;
};
