module;

#include "nlohmann/json.hpp"
#include <fstream>

export module preparser.jsonexport;
import preparser.types;

nlohmann::json property_value_to_json(const PropertyValue& value);

export void export_component(const ComponentInfo& component, const std::string& output_dir = ".") {
    nlohmann::json j;
    j["type"] = component.name;
    
    nlohmann::json values = nlohmann::json::object();
    for (const auto& prop : component.properties) {
        values[prop.name] = property_value_to_json(prop.value);
    }
    j["value"] = values;
    
    std::string filename = output_dir + "/" + component.name + ".component";
    std::ofstream file(filename);
    file << j.dump(4);
}

export void export_components(const std::vector<ComponentInfo>& components, const std::string& output_dir = ".") {
    for (const auto& comp : components) {
        export_component(comp, output_dir);
    }
}

nlohmann::json property_value_to_json(const PropertyValue& value) {
    return std::visit([]<typename T>(T&& v) -> nlohmann::json {
        return v;
    }, value);
}
