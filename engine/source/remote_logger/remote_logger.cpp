module;

#include <string>

module zeytin.logger;
import zeytin.common.message.engine_to_editor.log_message;
import zeytin.editor.communication;
import zeytin.editor.event;
import zeytin.editor.message;

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
	send_message_to_editor<LogMessage>(level_to_string(level), message);
}

