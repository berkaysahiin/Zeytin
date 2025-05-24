#pragma once

#include "common/property.h"
#include "rttr_enable.h"

struct IAction {
    IAction() = default;
    virtual void execute() = 0;
    bool is_executed = false;
    RTTR_ENABLE();
};

#define ACTION(ClassName) \
    RTTR_ENABLE(IAction); \
    static constexpr const char* get_action_name() { return #ClassName; } \
    public: \
