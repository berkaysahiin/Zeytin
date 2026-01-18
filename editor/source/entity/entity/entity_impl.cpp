module;

#include <algorithm>
#include <functional>

module zeytin.entity;

// WARNING: This function should log more

MaybeRef<ComponentInstance> add_component(Entity& entity, const ComponentID component_id) {
    if (has_component(entity, component_id)) {
        return {};
    }

    const MaybeRef<ComponentInstance> instance_opt = create_instance(component_id);
    if (!instance_opt) {
        return {};
    }

    const ComponentInstanceID instance_id = instance_opt->get().id;
    entity.components.push_back(instance_id);

    return instance_opt;
}

MaybeRef<ComponentInstance> get_or_add_component(Entity& entity, const ComponentID component_id) {
	MaybeRef<ComponentInstance> instance_opt = get_component_instance(entity, component_id);

	if(instance_opt) {
		return instance_opt;
	}
		
	return add_component(entity, component_id);
}

bool remove_component(Entity& entity, const ComponentID component_id) {
    auto it = std::ranges::find_if(entity.components, [component_id](const ComponentInstanceID id) {
        auto instance = get_instance_const(id);
        return instance && instance->get().component_id == component_id;
    });

    if (it == entity.components.end()) {
        return false;
    }

    destroy_instance(*it);
    entity.components.erase(it);
    return true;
}

bool has_component(const Entity& entity, const ComponentID component_id) {
    return std::ranges::any_of(entity.components, [component_id](const ComponentInstanceID id) {
        auto instance = get_instance_const(id);
        return instance && instance->get().component_id == component_id;
    });
}

MaybeRef<ComponentInstance> get_component_instance(Entity& entity, const ComponentID component_id) {
    auto it = std::ranges::find_if(entity.components, [component_id](const ComponentInstanceID id) {
        auto instance = get_instance_const(id);
        return instance && instance->get().component_id == component_id;
    });

    if (it == entity.components.end()) {
        return {};
    }

    return get_instance(*it);
}

MaybeRef<const ComponentInstance> get_component_instance(const Entity& entity, const ComponentID component_id) {
    auto it = std::ranges::find_if(entity.components, [component_id](const ComponentInstanceID id) {
        auto instance = get_instance_const(id);
        return instance && instance->get().component_id == component_id;
    });

    if (it == entity.components.end()) {
        return {};
    }

    return get_instance_const(*it);
}

List<Property> get_properties(const Entity& entity, const ComponentID component_id) {
    const MaybeRef<const ComponentInstance> instance_opt = get_component_instance(entity, component_id);
    if (!instance_opt) {
        return {};
    }
    
	return instance_get_properties(instance_opt->get());
}
