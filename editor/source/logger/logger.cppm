module;

#include <string>
#include <vector>
#include <mutex>
#include <functional>

export module zeytin.logger;
import zeytin.singleton;

export enum class LogLevel {
    TRACE,  
    INFO,
    WARNING,
    ERROR
};

export class Logger : public Singleton<Logger> {
	friend class Singleton<Logger>;
public:
    void log(LogLevel level, const std::string& message);
    
    const std::vector<std::pair<LogLevel, std::string>>& get_log_messages() const {
        return m_logs;
    }
    
    void register_callback(std::function<void(LogLevel, const std::string&)> callback) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_callbacks.push_back(callback);
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_logs.clear();
    }
    
    void set_min_log_level(LogLevel level) {
        m_min_log_level = level;
    }
    
    LogLevel get_min_log_level() const {
        return m_min_log_level;
    }
    
    static std::string level_to_string(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE: return "[TRACE]";  
            case LogLevel::INFO: return "[INFO]";
            case LogLevel::WARNING: return "[WARNING]";
            case LogLevel::ERROR: return "[ERROR]";
            default: return "[UNKNOWN]";
        }
    }

private:
    Logger() = default;

    std::vector<std::pair<LogLevel, std::string>> m_logs;
    std::vector<std::function<void(LogLevel, const std::string&)>> m_callbacks;
    std::mutex m_mutex;
    LogLevel m_min_log_level = LogLevel::TRACE;
};
