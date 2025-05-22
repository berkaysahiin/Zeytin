#pragma once

#define IN()
#define OUT()

struct IAction {
    virtual void execute() = 0;
    bool is_executed = false;
};
