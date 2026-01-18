module;

#include <string>
#include <vector>
#include "rapidjson/document.h"

export module zeytin.validation.component;

export struct RemovedProperty {
    std::string property_name;
    std::string property_path;
    rapidjson::Value value;
    
    RemovedProperty(std::string name, std::string path, rapidjson::Value&& val)
        : property_name(std::move(name))
        , property_path(std::move(path))
        , value(std::move(val))
    {}
};

export struct ComponentDifference {
    std::string component_type;
    std::vector<RemovedProperty> removed_properties;
    std::vector<std::string> added_properties;
    
    bool has_changes() const {
        return !removed_properties.empty() || !added_properties.empty();
    }
};

export namespace component_validation {

    ComponentDifference detect_component_differences(
        const std::string& component_type,
        rapidjson::Value& editor_component,
        const rapidjson::Value& engine_component,
        rapidjson::Document::AllocatorType& allocator);
}
