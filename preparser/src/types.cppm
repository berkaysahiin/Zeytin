module;

#include <string>
#include <utility>
#include <variant>
#include <vector>

export module preparser.types;

export using PropertyAttr = std::variant<std::string, std::pair<std::string, std::string>>;
export using PropertyValue = std::variant<std::string, float, int, bool>;

export struct PropertyInfo {
    std::string name;
	PropertyValue value; 
	std::vector<PropertyAttr> attrs;
	std::string annotation;
};

export struct ComponentInfo {
    std::string name;
    std::string module_name;
    std::vector<PropertyInfo> properties;
};

export std::vector<PropertyAttr> parse_attr_from_annotation(std::string_view annotation);

export void debug_print_component(const ComponentInfo& component);
export void debug_print_property(const PropertyInfo& component);
