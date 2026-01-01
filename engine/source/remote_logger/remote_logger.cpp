module;

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.logger;
import zeytin.editor.communication;
import zeytin.editor.event;

static std::string level_to_string(LogLevel level) {
	switch (level) {
		case LogLevel::TRACE: return "TRACE";  
		case LogLevel::INFO: return "INFO";
		case LogLevel::WARNING: return "WARNING";
		case LogLevel::ERROR: return "ERROR";
		default: return "UNKNOWN";
	}
}

void RemoteLogger::log(LogLevel level, const std::string& message) {
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

