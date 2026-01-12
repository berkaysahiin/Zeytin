module;

#include <cstdint>
#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.editor_to_engine.entity_variant_added;

import zeytin.common.message;

export struct EditorEntityVariantAddedMessage : public IMessage {
    uint64_t entity_id = 0;
    std::string variant_type;
    std::string component_json;

    EditorEntityVariantAddedMessage() = default;
    EditorEntityVariantAddedMessage(uint64_t id, std::string type, std::string json = {})
        : entity_id(id)
        , variant_type(std::move(type))
        , component_json(std::move(json)) {}

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
