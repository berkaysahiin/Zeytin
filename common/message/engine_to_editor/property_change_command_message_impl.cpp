module;

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.engine_to_editor.property_change_command;

std::string PropertyChangeCommandMessage::get_type() const {
    return "property_change_command";
}

std::string PropertyChangeCommandMessage::as_json() const {
    rapidjson::Document msg;
    msg.SetObject();
    auto& alloc = msg.GetAllocator();

    msg.AddMember("type", rapidjson::Value(get_type().c_str(), alloc), alloc);
    msg.AddMember("entity_id", entity_id, alloc);
    msg.AddMember("variant_type", rapidjson::Value(variant_type.c_str(), alloc), alloc);
    msg.AddMember("key_path", rapidjson::Value(key_path.c_str(), alloc), alloc);
    msg.AddMember("old_value", old_value, alloc);
    msg.AddMember("new_value", new_value, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    return buffer.GetString();
}

bool PropertyChangeCommandMessage::from_json(const rapidjson::Value& message) {
    if (!message.IsObject()) {
        return false;
    }

    if (!message.HasMember("type") || !message["type"].IsString()) {
        return false;
    }

    if (get_type() != message["type"].GetString()) {
        return false;
    }

    if (!message.HasMember("entity_id") || !message["entity_id"].IsUint64()) {
        return false;
    }

    if (!message.HasMember("variant_type") || !message["variant_type"].IsString()) {
        return false;
    }

    if (!message.HasMember("key_path") || !message["key_path"].IsString()) {
        return false;
    }

    if (!message.HasMember("old_value") || !message["old_value"].IsNumber()) {
        return false;
    }

    if (!message.HasMember("new_value") || !message["new_value"].IsNumber()) {
        return false;
    }

    entity_id = message["entity_id"].GetUint64();
    variant_type = message["variant_type"].GetString();
    key_path = message["key_path"].GetString();
    old_value = message["old_value"].GetFloat();
    new_value = message["new_value"].GetFloat();

    return true;
}
