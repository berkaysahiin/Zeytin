module;

#include <cstdint>
#include <string>
#include <vector>

#include "rapidjson/document.h"

export module zeytin.common.message.engine_to_editor.batch_property_change_command;

import zeytin.common.message;

export struct PropertyChangeEntry {
    std::string key_path;
    float old_value = 0.0f;
    float new_value = 0.0f;
};

export struct BatchPropertyChangeCommandMessage : public IMessage {
    uint64_t entity_id = 0;
    std::string variant_type;
    std::vector<PropertyChangeEntry> changes;

    BatchPropertyChangeCommandMessage() = default;
    BatchPropertyChangeCommandMessage(uint64_t id,
                                      std::string variant,
                                      std::vector<PropertyChangeEntry> entries)
        : entity_id(id)
        , variant_type(std::move(variant))
        , changes(std::move(entries)) {
    }

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
