module;

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.editor_to_engine.entity_variant_removed;

std::string EditorEntityVariantRemovedMessage::get_type() const {
    return "entity_variant_removed";
}

std::string EditorEntityVariantRemovedMessage::as_json() const {
    rapidjson::Document message_document;
    message_document.SetObject();
    auto& allocator = message_document.GetAllocator();

    message_document.AddMember("type", rapidjson::Value(get_type().c_str(), allocator), allocator);
    message_document.AddMember("entity_id", entity_id, allocator);
    message_document.AddMember("variant_type", rapidjson::Value(variant_type.c_str(), allocator), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    message_document.Accept(writer);

    return buffer.GetString();
}

bool EditorEntityVariantRemovedMessage::from_json(const rapidjson::Value& message) {
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

    entity_id = message["entity_id"].GetUint64();
    variant_type = message["variant_type"].GetString();

    return true;
}
