module;

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.engine_to_editor.enable_if_result;

std::string EnableIfResultMessage::get_type() const {
    return "enable_if_result";
}

std::string EnableIfResultMessage::as_json() const {
    rapidjson::Document message_document;
    message_document.SetObject();
    auto& allocator = message_document.GetAllocator();

    message_document.AddMember("type", rapidjson::Value(get_type().c_str(), allocator), allocator);
    message_document.AddMember("entity_id", entity_id, allocator);
    message_document.AddMember("variant_type", rapidjson::Value(variant_type.c_str(), allocator), allocator);
    message_document.AddMember("key_path", rapidjson::Value(key_path.c_str(), allocator), allocator);
    message_document.AddMember("enabled", enabled, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    message_document.Accept(writer);

    return buffer.GetString();
}

bool EnableIfResultMessage::from_json(const rapidjson::Value& message) {
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

    if (!message.HasMember("enabled") || !message["enabled"].IsBool()) {
        return false;
    }

    entity_id = message["entity_id"].GetUint64();
    variant_type = message["variant_type"].GetString();
    key_path = message["key_path"].GetString();
    enabled = message["enabled"].GetBool();

    return true;
}
