module;

#include <optional>
#include <functional>
#include <cstdint>

export module zeytin.entity.registry;
import zeytin.singleton;
import zeytin.entity.list;
import zeytin.entity.document;

export class EntityRegistry : public Singleton<EntityRegistry> {
    friend class Singleton<EntityRegistry>;
public:
    void set_entity_list(EntityList& entity_list);
    std::optional<std::reference_wrapper<EntityList>> get_entity_list() const;
    
    std::optional<std::reference_wrapper<EntityDocument>> find_entity(uint64_t entity_id);
    uint64_t generate_entity_id();

private:
    EntityRegistry() = default;
    
    EntityList* m_entity_list = nullptr;
};
