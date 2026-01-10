module;

#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.engine_to_editor.log_message;

import zeytin.common.message;

export struct LogMessage : public IMessage {
    std::string level;
    std::string message;

    LogMessage() = default;
    LogMessage(std::string level_value, std::string message_value)
        : level(std::move(level_value))
        , message(std::move(message_value)) {
    }

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message_value) override;
};
