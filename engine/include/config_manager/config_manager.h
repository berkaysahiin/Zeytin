#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <optional>
#include <filesystem>

#include "core/macros.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "remote_logger/remote_logger.h""

class ConfigManager {
    MAKE_SINGLETON(ConfigManager);

public:
    using ConfigValue = std::variant<
        int, 
        float, 
        bool, 
        std::string,
        std::filesystem::path
    >;

    bool load_config();
    bool save_config();

    template<typename T>
    void set(const std::string& key, T value) {
        m_config_values[key] = value;
    }

    template<typename T>
    T get(const std::string& key, T default_value = T()) const {
        auto it = m_config_values.find(key);
        if (it != m_config_values.end()) {
            try {
                std::cout << "[---------------] Config: " << key << " is found: " << std::get<int>(it->second) << std::endl;;
                return std::get<T>(it->second);
            } catch (const std::bad_variant_access&) {
                return default_value;
            }
        }
        std::cout << "Config: " << key << " is not found, returning default value: " << default_value;
        return default_value;
    }

    bool has(const std::string& key) const;
    void remove(const std::string& key);
    void clear();

private:
    ConfigManager() {
        load_config();
    }

    ~ConfigManager();

    std::unordered_map<std::string, ConfigValue> m_config_values;

    rapidjson::Value variant_to_json_value(const ConfigValue& value, rapidjson::Document::AllocatorType& allocator) const;
    ConfigValue json_value_to_variant(const rapidjson::Value& value) const;
};


#define CONFIG_GET(key, type, default_value) ConfigManager::get().get<type>(key, default_value)
#define CONFIG_SET(key, value) ConfigManager::get().set(key, value)
