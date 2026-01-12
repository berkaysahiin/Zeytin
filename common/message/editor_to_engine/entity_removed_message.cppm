module;

#include <cstdint>
#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.editor_to_engine.entity_removed;

import zeytin.common.message;

export struct EditorEntityRemovedMessage : public IMessage {
    uint64_t entity_id = 0;

    EditorEntityRemovedMessage() = default;
    explicit EditorEntityRemovedMessage(uint64_t id) : entity_id(id) {}

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
