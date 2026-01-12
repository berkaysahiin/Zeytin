module;

#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.editor_to_engine.window_state;

import zeytin.common.message;

export struct EditorWindowStateMessage : public IMessage {
    bool hidden = false;

    EditorWindowStateMessage() = default;
    explicit EditorWindowStateMessage(bool window_hidden) : hidden(window_hidden) {}

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
