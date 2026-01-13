module;

#include <cstdint>
#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.editor_to_engine.enable_if_request;

import zeytin.common.message;

export struct EnableIfRequestMessage : public IMessage {
    uint64_t entity_id = 0;
    std::string variant_type;
    std::string key_path;
    std::string method_name;

    EnableIfRequestMessage() = default;
    EnableIfRequestMessage(uint64_t id,
                           std::string variant,
                           std::string path,
                           std::string method)
        : entity_id(id)
        , variant_type(std::move(variant))
        , key_path(std::move(path))
        , method_name(std::move(method)) {
    }

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
