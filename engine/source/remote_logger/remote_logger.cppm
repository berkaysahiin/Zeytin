module;

#include <format>
#include <iostream>
#include <sstream>
#include <string>

export module zeytin.logger;

export enum class LogLevel {
    TRACE,  
    INFO,
    WARNING,
    ERROR
};

export class RemoteLogger;

export class RemoteLogStream {
public:
    RemoteLogStream(RemoteLogger& logger, LogLevel level);
    ~RemoteLogStream();
    
    template <typename T>
    RemoteLogStream& operator<<(const T& value) {
        m_stream << value;
        return *this;
    }
    
    typedef std::ostream& (*StreamManipulator)(std::ostream&);
    RemoteLogStream& operator<<(StreamManipulator manip) {
        manip(m_stream);
        return *this;
    }

private:
    std::ostringstream m_stream;
    RemoteLogger& m_logger;
    LogLevel m_level;
};

export class RemoteLogger {
public:
    static RemoteLogger& get() {
        static RemoteLogger instance;
        return instance;
    }
    
    RemoteLogStream trace() { return RemoteLogStream(*this, LogLevel::TRACE); }   
    RemoteLogStream info() { return RemoteLogStream(*this, LogLevel::INFO); }
    RemoteLogStream warning() { return RemoteLogStream(*this, LogLevel::WARNING); }
    RemoteLogStream error() { return RemoteLogStream(*this, LogLevel::ERROR); }
    
    void log(LogLevel level, const std::string& message);
    
    void set_min_log_level(LogLevel level) {
        m_min_log_level = level;
    }
    
    LogLevel get_min_log_level() const {
        return m_min_log_level;
    }
    
    static std::string level_to_string(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE: return "TRACE";  
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

private:
    RemoteLogger() : m_min_log_level(LogLevel::TRACE) {}
    ~RemoteLogger() = default;
    
    RemoteLogger(const RemoteLogger&) = delete;
    RemoteLogger& operator=(const RemoteLogger&) = delete;
    
    LogLevel m_min_log_level;
};


export template<typename... Args>
void log_trace(std::format_string<Args...> fmt, Args&&... args) {
    RemoteLogger::get().log(LogLevel::TRACE, 
        std::format(fmt, std::forward<Args>(args)...));
}

export template<typename... Args>
void log_info(std::format_string<Args...> fmt, Args&&... args) {
	std::string message = std::format(fmt, std::forward<Args>(args)...);
	std::cout << message << std::endl;
    RemoteLogger::get().log(LogLevel::INFO, message);
}

export template<typename... Args>
void log_warning(std::format_string<Args...> fmt, Args&&... args) {
    RemoteLogger::get().log(LogLevel::WARNING, 
        std::format(fmt, std::forward<Args>(args)...));
}

export template<typename... Args>
void log_error(std::format_string<Args...> fmt, Args&&... args) {
    RemoteLogger::get().log(LogLevel::ERROR, 
        std::format(fmt, std::forward<Args>(args)...));
}
