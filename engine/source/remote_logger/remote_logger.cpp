module;

#include <string>

module zeytin.logger;

#ifdef EDITOR_MODE
import zeytin.common.message.engine_to_editor.log_message;
import zeytin.editor.communication;
import zeytin.editor.event;
import zeytin.editor.message;
#endif

#ifdef EDITOR_MODE
static std::string level_to_string(LogLevel level) {
	switch (level) {
		case LogLevel::TRACE: return "TRACE";  
		case LogLevel::INFO: return "INFO";
		case LogLevel::WARNING: return "WARNING";
		case LogLevel::ERROR: return "ERROR";
		default: return "UNKNOWN";
	}
}
#endif

void RemoteLogger::log(
    [[maybe_unused]] LogLevel level,
    [[maybe_unused]] const std::string& message)
{
#ifdef EDITOR_MODE
	send_message_to_editor<LogMessage>(level_to_string(level), message);
#endif
}
