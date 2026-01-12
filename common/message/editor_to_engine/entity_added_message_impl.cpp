module;

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.editor_to_engine.entity_added;

std::string EditorEntityAddedMessage::get_type() const {
    return "entity_added";
}

std::string EditorEntityAddedMessage::as_json() const {
    rapidjson::Document message_document;
    message_document.SetObject();
    auto& allocator = message_document.GetAllocator();

    message_document.AddMember("type", rapidjson::Value(get_type().c_str(), allocator), allocator);
    message_document.AddMember("entity_id", entity_id, allocator);

    rapidjson::Document entity_document;
    if (!entity_json.empty()) {
        entity_document.Parse(entity_json.c_str());
    }

    if (!entity_json.empty() && !entity_document.HasParseError() && entity_document.IsObject()) {
        rapidjson::Value entity_copy;
        entity_copy.CopyFrom(entity_document, allocator);
        message_document.AddMember("entity_data", entity_copy, allocator);
    } else {
        message_document.AddMember("entity_data", rapidjson::Value(rapidjson::kObjectType), allocator);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    message_document.Accept(writer);

    return buffer.GetString();
}

bool EditorEntityAddedMessage::from_json(const rapidjson::Value& message) {
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

    if (!message.HasMember("entity_data") || !message["entity_data"].IsObject()) {
        return false;
    }

    entity_id = message["entity_id"].GetUint64();

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    message["entity_data"].Accept(writer);
    entity_json = buffer.GetString();

    return true;
}
