module;

#include <cstdint>
#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.entity_selected;

import zeytin.common.message;

export struct EntitySelectedMessage : public IMessage {
    uint64_t id = 0;

    EntitySelectedMessage() = default;
    explicit EntitySelectedMessage(uint64_t entity_id) : id(entity_id) {}

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
