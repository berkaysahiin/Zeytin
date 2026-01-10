module;

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.engine_started;

std::string EngineStartedMessage::get_type() const {
    return "engine_started";
}

std::string EngineStartedMessage::as_json() const {
    rapidjson::Document msg;
    msg.SetObject();
    auto& alloc = msg.GetAllocator();

    msg.AddMember("type", rapidjson::Value(get_type().c_str(), alloc), alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    return buffer.GetString();
}

bool EngineStartedMessage::from_json(const rapidjson::Value& message) {
    if (!message.IsObject()) {
        return false;
    }

    if (message.HasMember("type") && message["type"].IsString()) {
        return get_type() == message["type"].GetString();
    }

    return true;
}
