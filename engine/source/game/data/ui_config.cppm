module;

#include "preparser.h"

export module zeytin.game.ui_config;
import zeytin.component;

export struct GGameUIConfig final : public DataComponent
{
    PROPERTY(GROUP="Actions UI")
    float actions_x = 20.0F;

    PROPERTY(GROUP="Actions UI")
    float actions_y = 20.0F;

    PROPERTY(GROUP="Actions UI")
    int actions_font_size = 24;

    PROPERTY(GROUP="Actions UI")
    bool actions_anchor_right = true;

    PROPERTY(GROUP="Timer UI")
    float timer_x = 20.0F;

    PROPERTY(GROUP="Timer UI")
    float timer_y = 20.0F;

    PROPERTY(GROUP="Timer UI")
    int timer_font_size = 32;

    PROPERTY(GROUP="Mismatch UI")
    float mismatch_duration = 0.35F;

    PROPERTY(GROUP="Mismatch UI")
    float mismatch_shake_px = 3.0F;

    PROPERTY(GROUP="Mismatch UI")
    float mismatch_outline_thickness = 6.0F;

    PROPERTY(GROUP="Mismatch UI")
    int mismatch_color_r = 230;

    PROPERTY(GROUP="Mismatch UI")
    int mismatch_color_g = 80;

    PROPERTY(GROUP="Mismatch UI")
    int mismatch_color_b = 80;

    PROPERTY(GROUP="Mismatch UI")
    int mismatch_color_a = 240;
};
