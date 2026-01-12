module;

#include <cstdint>
#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.editor_to_engine.entity_variant_removed;

import zeytin.common.message;

export struct EditorEntityVariantRemovedMessage : public IMessage {
    uint64_t entity_id = 0;
    std::string variant_type;

    EditorEntityVariantRemovedMessage() = default;
    EditorEntityVariantRemovedMessage(uint64_t id, std::string type)
        : entity_id(id)
        , variant_type(std::move(type)) {}

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
