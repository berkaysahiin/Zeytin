#pragma once

#include "action/iaction.h"
#include "rttr/variant.h"

namespace rttr_json {
    rttr::variant deserialize_action(const std::string& action_json);
}


