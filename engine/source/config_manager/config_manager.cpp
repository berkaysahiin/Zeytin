module; 

#include "raylib.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#include <string>
#include <unordered_map>
#include <variant>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

module zeytin.config;
import zeytin.resource.manager;
import zeytin.logger;

static rapidjson::Value variant_to_json_value(const ConfigManager::ConfigValue& value, rapidjson::Document::AllocatorType& allocator);
static ConfigManager::ConfigValue json_value_to_variant(const rapidjson::Value& value);
static std::string variant_as_string(const ConfigManager::ConfigValue&);

static const char* config_file = "config.json";

struct ConfigManager::Impl 
{
	std::unordered_map<std::string, ConfigManager::ConfigValue> m_config_values;
};

ConfigManager::ConfigManager() : pImpl(new Impl()) {
	load_config();
}

ConfigManager::~ConfigManager() {

#ifdef EDITOR_MODE
    // we want to save window position in editor mode everytime
    const int screen_width = GetScreenWidth();
    const int screen_height = GetScreenHeight();
    const int window_x = GetWindowPosition().x;
    const int window_y = GetWindowPosition().y;

	set("screen_width", screen_width);
	set("screen_height", screen_height);
	set("window_x", window_x);
	set("window_y", window_y);
#endif

    save_config();
	clear();
	delete pImpl;
}

bool ConfigManager::load_config() {
    std::filesystem::path path = ResourceManager::get().get_resource_subdir("config") / config_file;

    if (!std::filesystem::exists(path)) {
		std::cout << "Config file not found: " << path << std::endl;
        return false;
    }

	std::cout << "Config file: " << path << std::endl;

    std::ifstream file(path);
    if (!file.is_open()) {
        //log_error() << "Failed to open config file: " << path << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string json_str = buffer.str();
    file.close();

    rapidjson::Document doc;
    rapidjson::ParseResult result = doc.Parse(json_str.c_str());

    if (result.IsError()) {
        //log_error() << "JSON parse error in config file" << std::endl;
        return false;
    }

    if (!doc.IsObject()) {
        //log_error() << "Config file must be a JSON object" << std::endl;
        return false;
    }

    pImpl->m_config_values.clear();

    for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
        std::string key = it->name.GetString();
        ConfigValue value = json_value_to_variant(it->value);
        pImpl->m_config_values[key] = value;
    }

	for(const auto& [key, value] : pImpl->m_config_values) {
		std::cout << "CONFIG " << key << " as: " << variant_as_string(value) << " " << std::endl;
	}

    return true;
}

bool ConfigManager::save_config() {
    std::filesystem::path path = ResourceManager::get().get_resource_subdir("config") / config_file;

    rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();

    for (const auto& [key, value] : pImpl->m_config_values) {
        rapidjson::Value json_key(key.c_str(), allocator);
        rapidjson::Value json_value = variant_to_json_value(value, allocator);
        doc.AddMember(json_key, json_value, allocator);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::ofstream file(path);
    if (!file.is_open()) {
        //log_error() << "Failed to open config file for writing: " << path << std::endl;
        return false;
    }

    file << buffer.GetString();
    file.close();

    return true;
}

void ConfigManager::set(const std::string& key, ConfigValue value) 
{
    pImpl->m_config_values[key] = value;
}

ConfigManager::MaybeConfig ConfigManager::get_impl(const std::string& key) const
{
	auto it = pImpl->m_config_values.find(key);
	if(it == pImpl->m_config_values.end()) {
		std::cout << "Cannot find config with name: " << key << std::endl;
		return {};
	}
	std::cout << "Found config with name: " << key << std::endl;
	return it->second;
}

bool ConfigManager::has(const std::string& key) const {
    auto it = pImpl->m_config_values.find(key);
	return it != pImpl->m_config_values.end();
}

void ConfigManager::remove(const std::string& key) {
    pImpl->m_config_values.erase(key);
}

void ConfigManager::clear() {
    pImpl->m_config_values.clear();
}

rapidjson::Value variant_to_json_value(const ConfigManager::ConfigValue& value, rapidjson::Document::AllocatorType& allocator) {
    rapidjson::Value json_value;

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            json_value.SetInt(arg);
        } else if constexpr (std::is_same_v<T, float>) {
            json_value.SetFloat(arg);
        } else if constexpr (std::is_same_v<T, bool>) {
            json_value.SetBool(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
            json_value.SetString(arg.c_str(), allocator);
        }
    }, value);

    return json_value;
}

ConfigManager::ConfigValue json_value_to_variant(const rapidjson::Value& value) {
    if (value.IsInt()) {
        return value.GetInt();
    } else if (value.IsFloat() || value.IsDouble()) {
        return value.GetFloat();
    } else if (value.IsBool()) {
        return value.GetBool();
    } else if (value.IsString()) {
        return std::string(value.GetString());
    }

    return std::string();
}


static std::string variant_as_string(const ConfigManager::ConfigValue& value)
{
	std::string rv;
 	std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
			rv = std::to_string(std::get<int>(value));
        } else if constexpr (std::is_same_v<T, float>) {
			rv = std::to_string(std::get<float>(value));
        } else if constexpr (std::is_same_v<T, bool>) {
			rv = std::to_string(std::get<bool>(value));
        } else if constexpr (std::is_same_v<T, std::string>) {
			rv = std::get<std::string>(value);
        }
    }, value);

	return rv;
}

