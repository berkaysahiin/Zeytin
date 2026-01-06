module;

#include "nlohmann/json.hpp"
#include <fstream>
#include <filesystem>

export module preparser.jsonexport;
import preparser.types;
import preparser.logger;

static nlohmann::json property_value_to_json(const PropertyValue& value);

export void export_component(const ComponentInfo& component) {
	// Skip components that don't require generation
	if (!component.requires_data_generation) {
		log("Up to date: {}", component.generated_component_file.string());
		return;
	}

    nlohmann::json j;
    j["type"] = component.name;

    nlohmann::json values = nlohmann::json::object();
    for (const auto& prop : component.properties) {
        values[prop.name] = property_value_to_json(prop.value);
    }
    j["value"] = values;

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
