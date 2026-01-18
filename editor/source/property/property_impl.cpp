module;

#include <type_traits>
#include <variant>
#include <string> // IWYU pragma: keep

module zeytin.property;

bool property_is_equal(const PropertyValue& lhs, const PropertyValue& rhs) {
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

bool property_is_null(const PropertyValue& value) {
    return std::holds_alternative<std::monostate>(value);
}
