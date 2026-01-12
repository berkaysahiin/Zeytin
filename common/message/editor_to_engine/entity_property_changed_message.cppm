module;

#include <cstdint>
#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.editor_to_engine.entity_property_changed;

import zeytin.common.message;

export struct EditorEntityPropertyChangedMessage : public IMessage {
    uint64_t entity_id = 0;
    std::string variant_type;
    std::string key_type;
    std::string key_path;
    std::string value;

    EditorEntityPropertyChangedMessage() = default;
    EditorEntityPropertyChangedMessage(uint64_t id,
                                       std::string variant,
                                       std::string keyType,
                                       std::string path,
                                       std::string newValue)
        : entity_id(id)
        , variant_type(std::move(variant))
        , key_type(std::move(keyType))
        , key_path(std::move(path))
        , value(std::move(newValue)) {}

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
