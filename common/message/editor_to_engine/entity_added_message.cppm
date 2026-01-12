module;

#include <cstdint>
#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.editor_to_engine.entity_added;

import zeytin.common.message;

export struct EditorEntityAddedMessage : public IMessage {
    uint64_t entity_id = 0;
    std::string entity_json;

    EditorEntityAddedMessage() = default;
    EditorEntityAddedMessage(uint64_t id, std::string json)
        : entity_id(id)
        , entity_json(std::move(json)) {}

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
