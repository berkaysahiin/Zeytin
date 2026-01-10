module;

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.entity_selected;

std::string EntitySelectedMessage::get_type() const {
    return "entity_selected_from_engine";
}

std::string EntitySelectedMessage::as_json() const {
    rapidjson::Document msg;
    msg.SetObject();
    auto& alloc = msg.GetAllocator();

    msg.AddMember("type", rapidjson::Value(get_type().c_str(), alloc), alloc);
    msg.AddMember("entity_id", id, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    return buffer.GetString();
}

bool EntitySelectedMessage::from_json(const rapidjson::Value& message) {
    if (!message.IsObject()) {
        return false;
    }

    if (message.HasMember("type") && message["type"].IsString()) {
        if (get_type() != message["type"].GetString()) {
            return false;
        }
    }

    if (!message.HasMember("entity_id") || !message["entity_id"].IsUint64()) {
        return false;
    }

    id = message["entity_id"].GetUint64();
    return true;
}
