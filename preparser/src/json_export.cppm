module;

#include "nlohmann/json.hpp"
#include <fstream>
#include <filesystem>

export module preparser.jsonexport;
import preparser.types;
import preparser.logger;

static nlohmann::json property_value_to_json(const PropertyValue& value);

export void export_component(const ComponentInfo& component) {
	if (!component.requires_data_generation) {
		log("Up to date: {}", component.generated_component_file.string());
		return;
	}

    nlohmann::json j;
    j["type"] = component.name;

    nlohmann::json values = nlohmann::json::object();
    nlohmann::json annotations_map = nlohmann::json::object();
    
    for (const auto& prop : component.properties) {
        // Add to old format
        values[prop.name] = property_value_to_json(prop.value);
        
        nlohmann::json prop_annotations = nlohmann::json::object();
        
        for (const auto& attr : prop.attrs) {
            if (std::holds_alternative<std::string>(attr)) {
                const std::string& flag = std::get<std::string>(attr);
                if (!flag.empty()) {
                	prop_annotations[flag] = true;
                }
            } else {
                const auto& [key, value] = std::get<std::pair<std::string, std::string>>(attr);
                if (!key.empty()) {
                    prop_annotations[key] = value;
                }
            }
        }
        
        // auto-add HIDDEN if SAVE is present
        if (prop_annotations.contains("SAVE") && prop_annotations["SAVE"] == true) {
            prop_annotations["HIDDEN"] = true;
        }

		if(!prop_annotations.empty()) {
			annotations_map[prop.name] = prop_annotations;
		}
    }
    
    j["value"] = values;
    j["annotations"] = annotations_map;

    std::ofstream file(component.generated_component_file);
    file << j.dump(4);
}

export void export_components(const std::vector<ComponentInfo>& components) {
    for (const auto& comp : components) {
        export_component(comp);
    }
}

static nlohmann::json property_value_to_json(const PropertyValue& value) {
    return std::visit([]<typename T>(T&& v) -> nlohmann::json {
        return v;
    }, value);
}
