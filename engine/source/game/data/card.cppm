module;

#include "preparser.h"

export module zeytin.game.card;
import zeytin.component;

export struct CCard final : public DataComponent
{
    PROPERTY(GROUP="Identity")
    int symbol_id = 0;

    PROPERTY(GROUP="Grid")
    int row = 0;

    PROPERTY(GROUP="Grid")
    int column = 0;

    PROPERTY(GROUP="State")
    bool is_face_up = false;

    PROPERTY(GROUP="State")
    bool is_matched = false;
};
