module;

#include <string>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.engine_to_editor.engine_shutdown;

std::string EngineShutdownMessage::get_type() const {
    return "engine_shutdown";
}

std::string EngineShutdownMessage::as_json() const {
    rapidjson::Document msg;
    msg.SetObject();
    auto& alloc = msg.GetAllocator();

    msg.AddMember("type", rapidjson::Value(get_type().c_str(), alloc), alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    return buffer.GetString();
}

bool EngineShutdownMessage::from_json(const rapidjson::Value& message) {
    if (!message.IsObject()) {
        return false;
    }

    if (message.HasMember("type") && message["type"].IsString()) {
        return get_type() == message["type"].GetString();
    }

    return true;
}
