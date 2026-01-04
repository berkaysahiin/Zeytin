module;

#include <optional>
#include <functional>

module zeytin.entity.registry;
import zeytin.logger;

void EntityRegistry::set_entity_list(EntityList& entity_list) {
    m_entity_list = &entity_list;
}

std::optional<std::reference_wrapper<EntityList>> EntityRegistry::get_entity_list() const {
    if (!m_entity_list) {
        return std::nullopt;
    }
    return std::ref(*m_entity_list);
}

void EntityRegistry::set_variant_list(VariantList& variant_list) {
    m_variant_list = &variant_list;
}

std::optional<std::reference_wrapper<VariantList>> EntityRegistry::get_variant_list() const {
    if (!m_variant_list) {
        return std::nullopt;
    }
    return std::ref(*m_variant_list);
}
