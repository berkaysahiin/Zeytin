module;

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.editor_to_engine.entity_variant_added;

std::string EditorEntityVariantAddedMessage::get_type() const {
    return "entity_variant_added";
}

std::string EditorEntityVariantAddedMessage::as_json() const {
    rapidjson::Document message_document;
    message_document.SetObject();
    auto& allocator = message_document.GetAllocator();

    message_document.AddMember("type", rapidjson::Value(get_type().c_str(), allocator), allocator);
    message_document.AddMember("entity_id", entity_id, allocator);
    message_document.AddMember("variant_type", rapidjson::Value(variant_type.c_str(), allocator), allocator);

    rapidjson::Document component_document;
    if (!component_json.empty()) {
        component_document.Parse(component_json.c_str());
    }

    if (!component_json.empty() && !component_document.HasParseError() && component_document.IsObject()) {
        rapidjson::Value component_copy;
        component_copy.CopyFrom(component_document, allocator);
        message_document.AddMember("component_data", component_copy, allocator);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    message_document.Accept(writer);

    return buffer.GetString();
}

bool EditorEntityVariantAddedMessage::from_json(const rapidjson::Value& message) {
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

    component_json.clear();
    if (message.HasMember("component_data")) {
        if (!message["component_data"].IsObject()) {
            return false;
        }

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        message["component_data"].Accept(writer);
        component_json = buffer.GetString();
    }

    return true;
}
