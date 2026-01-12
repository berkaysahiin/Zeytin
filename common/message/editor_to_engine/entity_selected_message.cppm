module;

#include <cstdint>
#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.editor_to_engine.entity_selected;

import zeytin.common.message;

export struct EditorEntitySelectedMessage : public IMessage {
    uint64_t entity_id = 0;

    EditorEntitySelectedMessage() = default;
    explicit EditorEntitySelectedMessage(uint64_t id) : entity_id(id) {}

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
