module;

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.logger;
import zeytin.editor.communication;
import zeytin.editor.event;

RemoteLogStream::RemoteLogStream(RemoteLogger& logger, LogLevel level) 
    : m_logger(logger), m_level(level) {
}

RemoteLogStream::~RemoteLogStream() {
    m_logger.log(m_level, m_stream.str());
}

void RemoteLogger::log(LogLevel level, const std::string& message) {
    if (level < m_min_log_level) {
        return;
    }
    
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    
    doc.AddMember("type", "log_message", allocator);
    
    rapidjson::Value level_str;
    std::string level_string = level_to_string(level);
    level_str.SetString(level_string.c_str(), level_string.length(), allocator);
    doc.AddMember("level", level_str, allocator);
    
    rapidjson::Value msg_str;
    msg_str.SetString(message.c_str(), message.length(), allocator);
    doc.AddMember("message", msg_str, allocator);
    
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    
    EditorEventBus::get().publish<const std::string&>(EditorEvent::LogToEditor, buffer.GetString());
}

