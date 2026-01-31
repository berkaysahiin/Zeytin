module;

#include "preparser.h"

export module zeytin.game.scoring_config;
import zeytin.component;

export struct GScoringConfig final : public DataComponent
{
    PROPERTY(GROUP="Scoring")
    int base_match_points = 10;

    PROPERTY(GROUP="Scoring")
    float multiplier_step = 0.5F;

    PROPERTY(GROUP="Scoring")
    float max_multiplier = 5.0F;

    PROPERTY(GROUP="Scoring")
    float sad_happy_bonus = 1.0F;
};
