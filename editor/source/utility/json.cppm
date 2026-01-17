module;

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
}
