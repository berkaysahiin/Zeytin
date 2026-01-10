module;

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.engine_to_editor.log_message;

std::string LogMessage::get_type() const {
    return "log_message";
}

std::string LogMessage::as_json() const {
    rapidjson::Document msg;
    msg.SetObject();
    auto& alloc = msg.GetAllocator();

    msg.AddMember("type", rapidjson::Value(get_type().c_str(), alloc), alloc);
    msg.AddMember("level", rapidjson::Value(level.c_str(), alloc), alloc);
    msg.AddMember("message", rapidjson::Value(message.c_str(), alloc), alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    return buffer.GetString();
}

bool LogMessage::from_json(const rapidjson::Value& message_value) {
    if (!message_value.IsObject()) {
        return false;
    }

    if (!message_value.HasMember("type") || !message_value["type"].IsString()) {
        return false;
    }

    if (get_type() != message_value["type"].GetString()) {
        return false;
    }

    if (!message_value.HasMember("level") || !message_value["level"].IsString()) {
        return false;
    }

    if (!message_value.HasMember("message") || !message_value["message"].IsString()) {
        return false;
    }

    level = message_value["level"].GetString();
    message = message_value["message"].GetString();

    return true;
}
