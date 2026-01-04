module;

#include <optional>
#include <functional>
#include <cstdint>

export module zeytin.entity.registry;
import zeytin.singleton;
import zeytin.entity.list;
import zeytin.variant.list;
import zeytin.entity.document;

export class EntityRegistry : public Singleton<EntityRegistry> {
    friend class Singleton<EntityRegistry>;
public:
    void set_entity_list(EntityList& entity_list);
    std::optional<std::reference_wrapper<EntityList>> get_entity_list() const;
    
    void set_variant_list(VariantList& variant_list);
    std::optional<std::reference_wrapper<VariantList>> get_variant_list() const;
    
    std::optional<std::reference_wrapper<EntityDocument>> find_entity(uint64_t entity_id);
    uint64_t generate_entity_id();

private:
    EntityRegistry() = default;
    
    EntityList* m_entity_list = nullptr;
    VariantList* m_variant_list = nullptr;
};
