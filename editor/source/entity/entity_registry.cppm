module;

#include <optional>
#include <functional>

export module zeytin.entity.registry;
import zeytin.entity.list;
import zeytin.singleton;
import zeytin.variant.list;

// Central registry for accessing the entity list
export class EntityRegistry : public Singleton<EntityRegistry> {
    friend class Singleton<EntityRegistry>;
public:
    void set_entity_list(EntityList& entity_list);
    std::optional<std::reference_wrapper<EntityList>> get_entity_list() const;

	void set_variant_list(VariantList& variant_list);
    std::optional<std::reference_wrapper<VariantList>> get_variant_list() const;

private:
    EntityRegistry() = default;
    EntityList* m_entity_list = nullptr;
	VariantList* m_variant_list = nullptr;
};
