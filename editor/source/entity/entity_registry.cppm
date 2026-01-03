module;

#include <optional>
#include <functional>

export module zeytin.entity.registry;
import zeytin.entity.list;
import zeytin.singleton;

// Central registry for accessing the entity list
export class EntityRegistry : public Singleton<EntityRegistry> {
    friend class Singleton<EntityRegistry>;
public:
    void set_entity_list(EntityList& entity_list);
    std::optional<std::reference_wrapper<EntityList>> get_entity_list() const;
private:
    EntityRegistry() = default;
    EntityList* m_entity_list = nullptr;
};
