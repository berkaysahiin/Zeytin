module;

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

#ifdef EDITOR_MODE

#define log_trace() RemoteLogger::get().trace()  
#define log_info() RemoteLogger::get().info()
#define log_warning() RemoteLogger::get().warning()
#define log_error() RemoteLogger::get().error()

#else

#define log_trace() 
#define log_info() 
#define log_warning() 
#define log_error() 

#endif


