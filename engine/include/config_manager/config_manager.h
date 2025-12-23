#pragma once

#include <string>
#include <variant>
#include <optional>

class ConfigManager {
public:
    using ConfigValue = 
		std::variant<int, float, bool, std::string>;
	using MaybeConfig = std::optional<std::reference_wrapper<const ConfigValue>>;

    ConfigManager();
    ~ConfigManager();

    bool load_config();
    bool save_config();

    bool has(const std::string& key) const;
    void remove(const std::string& key);
    void clear();

    void set(const std::string& key, ConfigValue value);

	template<typename T>
	T get_or(const std::string& key, T default_value) const
	{
		MaybeConfig maybe_config = get_impl(key);
		if(!maybe_config) {
			return default_value;
		}
		return std::get<T>(maybe_config.value().get());
	}

private:
	MaybeConfig get_impl(const std::string& key) const;

	struct Impl;
	Impl* pImpl;
};
