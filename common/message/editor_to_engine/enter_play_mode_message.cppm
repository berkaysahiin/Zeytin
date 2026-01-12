module;

#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.editor_to_engine.enter_play_mode;

import zeytin.common.message;

export struct EditorEnterPlayModeMessage : public IMessage {
    bool is_paused = false;

    EditorEnterPlayModeMessage() = default;
    explicit EditorEnterPlayModeMessage(bool paused) : is_paused(paused) {}

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
