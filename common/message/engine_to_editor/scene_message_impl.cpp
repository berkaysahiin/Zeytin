module;

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.engine_to_editor.scene;

std::string SceneMessage::get_type() const {
    return "scene";
}

std::string SceneMessage::as_json() const {
    if (!scene_json.empty()) {
        return scene_json;
    }

    rapidjson::Document msg;
    msg.SetObject();
    auto& alloc = msg.GetAllocator();

    msg.AddMember("type", rapidjson::Value(get_type().c_str(), alloc), alloc);
    msg.AddMember("entities", rapidjson::Value(rapidjson::kArrayType), alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    return buffer.GetString();
}

bool SceneMessage::from_json(const rapidjson::Value& message) {
    if (!message.IsObject()) {
        return false;
    }

    if (!message.HasMember("type") || !message["type"].IsString()) {
        return false;
    }

    if (get_type() != message["type"].GetString()) {
        return false;
    }

    if (!message.HasMember("entities") || !message["entities"].IsArray()) {
        return false;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    message.Accept(writer);
    scene_json = buffer.GetString();

    return true;
}
