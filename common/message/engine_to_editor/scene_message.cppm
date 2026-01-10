module;

#include <string>
#include "rapidjson/document.h"

export module zeytin.common.message.engine_to_editor.scene;

import zeytin.common.message;

export struct SceneMessage : public IMessage {
    std::string scene_json;

    SceneMessage() = default;
    explicit SceneMessage(std::string scene) : scene_json(std::move(scene)) {}

    std::string get_type() const override;
    std::string as_json() const override;
    bool from_json(const rapidjson::Value& message) override;
};
