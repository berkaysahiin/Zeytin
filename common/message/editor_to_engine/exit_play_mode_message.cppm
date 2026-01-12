module;

#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.editor_to_engine.exit_play_mode;

import zeytin.common.message;

export struct EditorExitPlayModeMessage : public IMessage {
    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
