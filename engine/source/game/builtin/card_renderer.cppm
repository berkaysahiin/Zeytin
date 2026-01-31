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

    PROPERTY(GROUP="Symbol Textures")
    std::string symbol_0_path = "symbol_0.png";

    PROPERTY(GROUP="Symbol Textures")
    std::string symbol_1_path = "symbol_1.png";

    PROPERTY(GROUP="Symbol Textures")
    std::string symbol_2_path = "symbol_2.png";

    PROPERTY(GROUP="Symbol Textures")
    std::string symbol_3_path = "symbol_3.png";

    PROPERTY(GROUP="Symbol Textures")
    std::string symbol_4_path = "symbol_4.png";

    void on_update() override;
};
