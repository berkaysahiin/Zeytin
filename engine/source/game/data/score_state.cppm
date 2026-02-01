module;

#include "preparser.h"
#include <string>

export module zeytin.game.score_state;
import zeytin.component;

export struct GScoreState final : public DataComponent
{
    PROPERTY(GROUP="Score")
    int current_points = 0;

    PROPERTY(GROUP="Score")
    int target_points = 120;

    PROPERTY(GROUP="Score")
    float multiplier = 1.0F;

    PROPERTY(GROUP="Progression")
    int level_index = 1;

    PROPERTY(GROUP="Progression")
    std::string next_level;
};
