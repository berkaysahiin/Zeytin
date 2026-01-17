module;

#include <string>
#include <cstdint>

export module zeytin.property;
import zeytin.utility.json;

export {
    using PropertyValue = JsonValue;
	using PropertyName = std::string;
	using PropertyID = uint64_t;

 	struct Property {
        PropertyName name;
        PropertyValue value;
		PropertyID id;
    };
}

