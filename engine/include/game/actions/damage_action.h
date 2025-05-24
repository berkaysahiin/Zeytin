#pragma once

#include "action/iaction.h"
#include "entity/entity.h"

#include "remote_logger/remote_logger.h"

struct DamageAction : IAction {
    ACTION(DamageAction);

    float amount; PROPERTY(IN);
    entity_id entity; PROPERTY(OUT)

    std::string died; PROPERTY(OUT)
    
    virtual void execute() override {
        log_info() << "Executing DamageAction on entity: " << entity << " with amount: " << amount << std::endl;
        died = false;
    }
};
