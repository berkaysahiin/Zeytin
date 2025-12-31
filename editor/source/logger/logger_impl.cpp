module;

#include <iostream>

module zeytin.logger;

void Logger::log(LogLevel level, const std::string& message) {
    if (level < m_min_log_level) {
        return;
    }
    
    std::string formattedMessage = level_to_string(level) + " " + message;

    if(level == LogLevel::ERROR) {
        std::cerr << formattedMessage << std::endl;
    }
    else {
        std::cout << formattedMessage << std::endl;
    }
    
    {
        m_logs.emplace_back(level, formattedMessage);
        
        constexpr size_t MAX_LOGS = 10000;
        if (m_logs.size() > MAX_LOGS) {
            m_logs.erase(m_logs.begin(), m_logs.begin() + (m_logs.size() - MAX_LOGS));
        }
        
        for (const auto& callback : m_callbacks) {
            callback(level, formattedMessage);
        }
    }
}
