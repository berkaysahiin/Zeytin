module;

#include <cstdint>
#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.engine_to_editor.enable_if_result;

import zeytin.common.message;

export struct EnableIfResultMessage : public IMessage {
    uint64_t entity_id = 0;
    std::string variant_type;
    std::string key_path;
    bool enabled = false;

    EnableIfResultMessage() = default;
    EnableIfResultMessage(uint64_t id,
                          std::string variant,
                          std::string path,
                          bool is_enabled)
        : entity_id(id)
        , variant_type(std::move(variant))
        , key_path(std::move(path))
        , enabled(is_enabled) {
    }

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
