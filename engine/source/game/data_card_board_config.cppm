module;

#include "preparser.h"
#include <cstdint>

export module zeytin.game.card_board_config;
import zeytin.component;

export struct GCardBoardConfig final : public DataComponent
{
    PROPERTY(GROUP="Grid")
    int32_t rows = 4;

    PROPERTY(GROUP="Grid")
    int32_t columns = 4;


    PROPERTY(GROUP="Canvas")
    float canvas_x = 0.0F;

    PROPERTY(GROUP="Canvas")
    float canvas_y = 0.0F;

    PROPERTY(GROUP="Canvas")
    float canvas_width = 1920.0F;

    PROPERTY(GROUP="Canvas")
    float canvas_height = 1080.0F;

    PROPERTY(GROUP="Canvas")
    bool use_virtual_canvas = false;

    PROPERTY(GROUP="Canvas")
    bool canvas_centered = false;

    PROPERTY(GROUP="Debug")
    bool draw_canvas_outline = true;

    PROPERTY(GROUP="Debug")
    bool draw_cell_outline = false;
};
