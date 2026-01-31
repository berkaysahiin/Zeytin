module;

#include "preparser.h"
#include <string>

export module zeytin.game.card_renderer;
import zeytin.component;

export struct CCardRenderer final : public Component
{
    PROPERTY(GROUP="Colors")
    int card_red = 140;

    PROPERTY(GROUP="Colors")
    int card_green = 140;

    PROPERTY(GROUP="Colors")
    int card_blue = 140;

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

    PROPERTY(GROUP="Mask Textures")
    std::string mask_happy_path = "happy_face.png";

    PROPERTY(GROUP="Mask Textures")
    std::string mask_sad_path = "sad_face.png";

    void on_update() override;
};
