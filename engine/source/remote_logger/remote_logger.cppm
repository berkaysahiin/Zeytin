module;

#include <format>
#include <iostream>
#include <string>

export module zeytin.logger;
import zeytin.singleton;

enum class LogLevel {
    TRACE,  
    INFO,
    WARNING,
    ERROR
};

class RemoteLogger : public Singleton<RemoteLogger> {
	friend class Singleton<RemoteLogger>;
public:
    void log(LogLevel level, const std::string& message);

private:
    RemoteLogger() = default;
    ~RemoteLogger() = default;
    
    RemoteLogger(const RemoteLogger&) = delete;
    RemoteLogger& operator=(const RemoteLogger&) = delete;
};

export template<typename... Args>
void log_trace(std::format_string<Args...> fmt, Args&&... args) {
	std::string message = std::format(fmt, std::forward<Args>(args)...);
	std::cout << message << std::endl;
    RemoteLogger::get().log(LogLevel::TRACE, message);
}

export template<typename... Args>
void log_info(std::format_string<Args...> fmt, Args&&... args) {
	std::string message = std::format(fmt, std::forward<Args>(args)...);
	std::cout << message << std::endl;
    RemoteLogger::get().log(LogLevel::INFO, message);
}

export template<typename... Args>
void log_warning(std::format_string<Args...> fmt, Args&&... args) {
	std::string message = std::format(fmt, std::forward<Args>(args)...);
	std::cout << message << std::endl;
    RemoteLogger::get().log(LogLevel::WARNING, message);
}

export template<typename... Args>
void log_error(std::format_string<Args...> fmt, Args&&... args) {
	std::string message = std::format(fmt, std::forward<Args>(args)...);
	std::cout << message << std::endl;
    RemoteLogger::get().log(LogLevel::ERROR, message);
}
