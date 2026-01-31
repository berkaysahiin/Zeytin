module;

#include "preparser.h"
#include <string>

export module zeytin.game.card_renderer;
import zeytin.component;

export struct CCardRenderer final : public Component
{
    PROPERTY(GROUP="Mask Textures")
    std::string mask_happy_path = "card_smile_masked.png";

    PROPERTY(GROUP="Mask Textures")
    std::string mask_sad_path = "card_sad_masked.png";

    void on_update() override;
};
