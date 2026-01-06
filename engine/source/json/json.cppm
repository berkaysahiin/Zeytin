module;

#include <vector>
#include <filesystem>

#include "rttr/variant.h"

export module zeytin.json;
import zeytin.entity;

namespace rttr_json {
    export EntityID deserialize_entity(const std::string& entity_json, EntityID& entity, std::vector<rttr::variant>& variants);
    export std::string serialize_entity(const EntityID entity_id, const std::vector<rttr::variant>& variants);
    export std::string serialize_entity(const EntityID entity_id, const std::vector<rttr::variant>& variants, const std::filesystem::path& path);
    export rttr::variant deserialize_component(EntityID entity_id, const std::string& component_json);
}
