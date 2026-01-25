export module zeytin.entity;

import zeytin.component;
import zeytin.component.instance;
import zeytin.component.registry;
import zeytin.common.guid;
import zeytin.utility.typedefs;
import zeytin.property;

export {
    using EntityID = uint64;

    struct Entity {
        EntityID id;
        String name;
        List<ComponentInstanceID> components; // We could to this ComponentInstance ?
    };

	[[nodiscard]]
    MaybeRef<ComponentInstance> add_component(Entity&, ComponentID); 

	[[nodiscard]]
    MaybeRef<ComponentInstance> get_or_add_component(Entity&, ComponentID);

	[[nodiscard]]
    bool remove_component(Entity& entity, ComponentID component_id);

	[[nodiscard]]
    bool has_component(const Entity& entity, ComponentID component_id);

    MaybeRef<ComponentInstance> get_component_instance(Entity& entity, ComponentID component_id);
    MaybeRef<const ComponentInstance> get_component_instance(const Entity& entity, ComponentID component_id);
    
    List<Property> get_properties(const Entity& entity, ComponentID component_id);
}
