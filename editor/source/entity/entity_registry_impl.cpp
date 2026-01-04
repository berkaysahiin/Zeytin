module;

#include <optional>
#include <functional>
#include <cstdint>
#include <random>

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

std::optional<std::reference_wrapper<EntityDocument>> EntityRegistry::find_entity(uint64_t entity_id) {
    if (!m_entity_list) {
        log_error("EntityRegistry: EntityList not registered");
        return std::nullopt;
    }
    
    return m_entity_list->find_entity_by_id(entity_id);
}

uint64_t EntityRegistry::generate_entity_id() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    return dis(gen);
}
