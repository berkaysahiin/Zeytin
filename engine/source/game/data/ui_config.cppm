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
    float actions_effect_duration = 0.25F;

    PROPERTY(GROUP="Actions UI")
    float actions_shake_px = 3.0F;

    PROPERTY(GROUP="Actions UI")
    int actions_flash_r = 230;

    PROPERTY(GROUP="Actions UI")
    int actions_flash_g = 90;

    PROPERTY(GROUP="Actions UI")
    int actions_flash_b = 90;

    PROPERTY(GROUP="Selection UI")
    float invalid_select_duration = 0.2F;

    PROPERTY(GROUP="Selection UI")
    float invalid_select_shake_px = 2.0F;

    PROPERTY(GROUP="Selection UI")
    float invalid_select_outline_thickness = 4.0F;

    PROPERTY(GROUP="Selection UI")
    int invalid_select_color_r = 255;

    PROPERTY(GROUP="Selection UI")
    int invalid_select_color_g = 160;

    PROPERTY(GROUP="Selection UI")
    int invalid_select_color_b = 80;

    PROPERTY(GROUP="Selection UI")
    int invalid_select_color_a = 220;

    PROPERTY(GROUP="Score UI")
    float score_panel_x = 0.0F;

    PROPERTY(GROUP="Score UI")
    float score_panel_y = -1.0F;

    PROPERTY(GROUP="Score UI")
    float score_panel_width = 0.0F;

    PROPERTY(GROUP="Score UI")
    float score_panel_height = 140.0F;

    PROPERTY(GROUP="Multiplier UI")
    float multiplier_fx_duration = 0.3F;

    PROPERTY(GROUP="Multiplier UI")
    float multiplier_fx_shake_px = 3.0F;

    PROPERTY(GROUP="Multiplier UI")
    int multiplier_fx_color_r = 255;

    PROPERTY(GROUP="Multiplier UI")
    int multiplier_fx_color_g = 140;

    PROPERTY(GROUP="Multiplier UI")
    int multiplier_fx_color_b = 60;

    PROPERTY(GROUP="Multiplier UI")
    int multiplier_fx_color_a = 160;

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

    PROPERTY(GROUP="Mismatch UI")
    float mismatch_trail_thickness = 6.0F;

    PROPERTY(GROUP="Mismatch UI")
    float mismatch_camera_shake_px = 6.0F;

    PROPERTY(GROUP="Mismatch UI")
    float mismatch_camera_shake_duration = 0.2F;

    PROPERTY(GROUP="Move UI")
    float move_duration = 0.25F;

    PROPERTY(GROUP="Move UI")
    float move_outline_thickness = 5.0F;

    PROPERTY(GROUP="Move UI")
    int move_color_r = 90;

    PROPERTY(GROUP="Move UI")
    int move_color_g = 200;

    PROPERTY(GROUP="Move UI")
    int move_color_b = 255;

    PROPERTY(GROUP="Move UI")
    int move_color_a = 220;

    PROPERTY(GROUP="Match UI")
    float match_trail_duration = 0.35F;

    PROPERTY(GROUP="Match UI")
    float match_trail_thickness = 6.0F;

    PROPERTY(GROUP="Match UI")
    int match_color_r = 120;

    PROPERTY(GROUP="Match UI")
    int match_color_g = 230;

    PROPERTY(GROUP="Match UI")
    int match_color_b = 140;

    PROPERTY(GROUP="Match UI")
    int match_color_a = 220;
};
