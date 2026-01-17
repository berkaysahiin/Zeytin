module;

#include "rapidjson/document.h"

#include <variant>
#include <cstdint>
#include <string>

export module zeytin.utility.json;

export {
	using JsonValue = std::variant<
		std::monostate,
        bool,
        int64_t,
        double,
        std::string
    >;

 	JsonValue json_value_from_rapidjson(const rapidjson::Value& value) {
        if (value.IsNull()) {
            return std::monostate{};
        }
        if (value.IsBool()) {
            return value.GetBool();
        }
        if (value.IsInt64()) {
            return value.GetInt64();
        }
        if (value.IsDouble()) {
            return value.GetDouble();
        }
        if (value.IsString()) {
            return std::string{value.GetString()};
        }
        return std::monostate{};
    }

    rapidjson::Value rapidjson_value_from_json(const JsonValue& value, rapidjson::Document::AllocatorType& allocator) {
        return std::visit([&allocator](const auto& val) -> rapidjson::Value {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return rapidjson::Value{};
            } else if constexpr (std::is_same_v<T, bool>) {
                return rapidjson::Value{val};
            } else if constexpr (std::is_same_v<T, int64_t>) {
                return rapidjson::Value{val};
            } else if constexpr (std::is_same_v<T, double>) {
                return rapidjson::Value{val};
            } else if constexpr (std::is_same_v<T, std::string>) {
                return rapidjson::Value{val.c_str(), static_cast<rapidjson::SizeType>(val.size()), allocator};
            }
            return rapidjson::Value{};
        }, value);
    }
}
