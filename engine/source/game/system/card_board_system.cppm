module;

#include "preparser.h"

export module zeytin.game.card_board_system;
import zeytin.component;

export struct CCardBoardSystem final : public Component
{
    PROPERTY(GROUP="Card Size")
    float initial_collider_width = 96.0F;

    PROPERTY(GROUP="Card Size")
    float initial_collider_height = 128.0F;

    void on_play_start() override;
};
