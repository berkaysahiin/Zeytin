module;

#include <string>
#include <utility>
#include <variant>
#include <vector>

export module preparser.types;

using PropertyAttr = std::variant<std::string, std::pair<std::string, std::string>>;

export struct PropertyInfo {
    std::string name;
    std::string type;
	std::vector<PropertyAttr> attrs;
};

export struct ComponentInfo {
    std::string name;
    std::vector<PropertyInfo> properties;
};

export std::vector<PropertyAttr> parse_attr_from_annotation(std::string_view annotation);

export void debug_print_component(const ComponentInfo& component);
export void debug_print_property(const PropertyInfo& component);
