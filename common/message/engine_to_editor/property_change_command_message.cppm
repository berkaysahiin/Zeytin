module;

#include <cstdint>
#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.engine_to_editor.property_change_command;

import zeytin.common.message;

export struct PropertyChangeCommandMessage : public IMessage {
    uint64_t entity_id = 0;
    std::string variant_type;
    std::string key_path;
    float old_value = 0.0f;
    float new_value = 0.0f;

    PropertyChangeCommandMessage() = default;
    PropertyChangeCommandMessage(uint64_t id,
                                 std::string variant,
                                 std::string key,
                                 float old_val,
                                 float new_val)
        : entity_id(id)
        , variant_type(std::move(variant))
        , key_path(std::move(key))
        , old_value(old_val)
        , new_value(new_val) {
    }

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
