module;

#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <sstream>

module preparser.types;
import preparser.logger;

static void ltrim(std::string &s) {
    s.erase(s.begin(), std::ranges::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

static void rtrim(std::string &s) {
    s.erase(std::ranges::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

std::vector<PropertyAttr> parse_attr_from_annotation(const std::string_view annotation) {
	std::vector<std::string> comma_seperated;
	std::stringstream ss(annotation.data());

    while (ss.good()) {
		std::string substr;
		std::getline(ss, substr, ',');
		trim(substr);
        comma_seperated.push_back(substr);
    }

	std::vector<PropertyAttr> return_value;
	return_value.reserve(comma_seperated.size());

    for (const auto& attr_str : comma_seperated) {
		size_t equal_sign = attr_str.find('=');
		if(equal_sign == std::string::npos) {
			return_value.push_back(attr_str);
		}
		else {
			std::string key = attr_str.substr(0, equal_sign);
			std::string value = attr_str.substr(equal_sign + 1);
			std::pair<std::string, std::string> pair = {key, value};
			return_value.push_back(pair);
		}
	}

	return return_value;
}

void debug_print_property(const PropertyInfo& property)
{
	const std::string property_value = std::visit([]<typename T>(T&& v) -> std::string {
    	if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
    	    return v ? "true" : "false";
    	} else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
    	    return v;
    	} else {
    	    return std::to_string(v);
    	}
	}, property.value);
	
	log("{} : {} | {}", property.name, property_value, property.annotation);
}

void debug_print_component(const ComponentInfo& component) {
	//auto visitor = [](auto&& arg) -> std::string { 
    //    using T = std::decay_t<decltype(arg)>; // Get the underlying type of 'arg'

    //    // Check if 'arg' is an int at compile-time
    //    if constexpr (std::is_same_v<T, int>) {
    //        return "It's an int: " + std::to_string(arg); // Convert int to string and return
    //    } 
    //    // Check if 'arg' is a string at compile-time
    //    else if constexpr (std::is_same_v<T, std::string>) {
    //        return "It's a string: " + arg; // Concatenate and return
    //    } 
    //};

	log("Component: {}", component.name);
	for(const auto& property : component.properties) {
		debug_print_property(property);
	}
}
