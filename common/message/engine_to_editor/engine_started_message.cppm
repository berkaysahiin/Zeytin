module;

#include <string>
#include "rapidjson/document.h"

export module zeytin.common.message.engine_to_editor.engine_started;

import zeytin.common.message;

export struct EngineStartedMessage : public IMessage {
    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
