module;

#include <string>
#include <cstdint>

export module zeytin.property;
import zeytin.utility.json;
import zeytin.utility.typedefs;

export {
    using PropertyValue = JsonValue;
	using PropertyName = std::string;
	using PropertyID = uint64_t;

 	struct Property {
        PropertyName name;
        PropertyValue value;
		PropertyID id;
    };

	bool property_is_equal(ConstRef<PropertyValue>, ConstRef<PropertyValue>);
    bool property_is_null(ConstRef<PropertyValue>);
    String property_value_to_string(ConstRef<PropertyValue> value);
}

