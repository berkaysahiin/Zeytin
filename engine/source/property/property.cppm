module;

#include <vector>
#include <string>
#include <sstream>
#include <optional>

#include "rttr/variant.h"

export module zeytin.property;

export template<typename T>
void update_property(rttr::variant& obj, const std::vector<std::string>& path_parts, const size_t path_index, const T& value) {
	if (path_index >= path_parts.size()) {
		return; 
	}

	const std::string& current_path = path_parts[path_index];

	if (path_index == path_parts.size() - 1) {
		for (auto& property : obj.get_type().get_properties()) {
			if (property.get_name() == current_path) {
				property.set_value(obj, value);

				// invoke set callback
				const auto callback = property.get_metadata("SET_CALLBACK");
				if(!callback.is_valid()) {
					return;
				}

				const std::string set_callback_name = callback.to_string();

				const rttr::method set_callback = obj.get_type().get_method(set_callback_name);;
				if(set_callback.is_valid()) {
					set_callback.invoke(obj);
				}

				return;
			}
		}
	} else {
		for (auto& property : obj.get_type().get_properties()) {
			if (property.get_name() == current_path) {
				rttr::variant nested_obj = property.get_value(obj);
				update_property(nested_obj, path_parts, path_index + 1, value);
				property.set_value(obj, nested_obj);
				return;
			}
		}
	}
}

export std::optional<rttr::variant> get_property_value(rttr::variant& obj,
                                                      const std::vector<std::string>& path_parts,
                                                      const size_t path_index) {
    if (path_index >= path_parts.size()) {
        return std::nullopt;
    }

    const std::string& current_path = path_parts[path_index];

    for (auto& property : obj.get_type().get_properties()) {
        if (property.get_name() != current_path) {
            continue;
        }

        rttr::variant value = property.get_value(obj);
        if (path_index == path_parts.size() - 1) {
            return value;
        }

        return get_property_value(value, path_parts, path_index + 1);
    }

    return std::nullopt;
}

export std::vector<std::string> split_path(const std::string& path) {
    std::vector<std::string> path_parts;
    std::string current_part;
    std::istringstream path_stream(path);

    while (std::getline(path_stream, current_part, '.')) {
        path_parts.push_back(current_part);
    }

    return path_parts;
}



