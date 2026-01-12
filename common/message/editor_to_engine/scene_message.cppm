module;

#include <string>

#include "rapidjson/document.h"

export module zeytin.common.message.editor_to_engine.scene;

import zeytin.common.message;

export struct EditorSceneMessage : public IMessage {
    std::string scene_json;

    EditorSceneMessage() = default;
    explicit EditorSceneMessage(std::string json) : scene_json(std::move(json)) {}

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
