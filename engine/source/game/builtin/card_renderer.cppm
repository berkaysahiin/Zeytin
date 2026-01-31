module;

#include "preparser.h"

export module zeytin.game.card_renderer;
import zeytin.component;

export struct CCardRenderer final : public Component
{
    PROPERTY(GROUP="Colors")
    int card_red = 160;

    PROPERTY(GROUP="Colors")
    int card_green = 100;

    PROPERTY(GROUP="Colors")
    int card_blue = 60;

    PROPERTY(GROUP="Colors")
    int card_alpha = 255;

    PROPERTY(GROUP="Colors")
    int face_red = 0;

    PROPERTY(GROUP="Colors")
    int face_green = 0;

    PROPERTY(GROUP="Colors")
    int face_blue = 0;

    PROPERTY(GROUP="Colors")
    int face_alpha = 255;

    void on_update() override;
};
