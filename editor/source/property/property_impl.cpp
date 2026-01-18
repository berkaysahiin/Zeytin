module;

#include <type_traits>
#include <variant>
#include <string> // IWYU pragma: keep

module zeytin.property;

bool property_is_equal(ConstRef<PropertyValue> lhs, ConstRef<PropertyValue> rhs) {
    if (lhs.index() != rhs.index()) {
        return false;
    }
    return std::visit([](const auto& a, const auto& b) {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;
        if constexpr (!std::is_same_v<A, B>) {
            return false;
        } else {
            return a == b;
        }
    }, lhs, rhs);
}

bool property_is_null(ConstRef<PropertyValue> value) {
    return std::holds_alternative<std::monostate>(value);
}

String property_value_to_string(ConstRef<PropertyValue> value) {
	return std::visit([](const auto& val) -> std::string {
		using T = std::decay_t<decltype(val)>;
		if constexpr (std::is_same_v<T, std::monostate>) {
			return "(null)";
		} else if constexpr (std::is_same_v<T, bool>) {
			return val ? "true" : "false";
		} else if constexpr (std::is_same_v<T, int64_t>) {
			return std::to_string(val);
		} else if constexpr (std::is_same_v<T, double>) {
			return std::to_string(val);
		} else if constexpr (std::is_same_v<T, std::string>) {
			return val;
		}
		return "[ERROR]";
	}, value);
}
