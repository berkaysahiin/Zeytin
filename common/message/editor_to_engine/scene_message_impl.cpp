module;

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.editor_to_engine.scene;

std::string EditorSceneMessage::get_type() const {
    return "scene";
}

std::string EditorSceneMessage::as_json() const {
    if (!scene_json.empty()) {
        return scene_json;
    }

    rapidjson::Document message_document;
    message_document.SetObject();
    auto& allocator = message_document.GetAllocator();

    message_document.AddMember("type", rapidjson::Value(get_type().c_str(), allocator), allocator);
    message_document.AddMember("entities", rapidjson::Value(rapidjson::kArrayType), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    message_document.Accept(writer);

    return buffer.GetString();
}

bool EditorSceneMessage::from_json(const rapidjson::Value& message) {
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
