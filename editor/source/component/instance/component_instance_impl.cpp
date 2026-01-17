module;

#include <utility>
#include <functional>

module zeytin.component.instance;

ComponentInstance create_component_instance(const ComponentID component_id) {
    ComponentInstance instance;
    instance.component_id = component_id;
    instance.id = generate_unique_id();
    return instance;
}

void set_override(ComponentInstance& instance, const PropertyID property_id, PropertyValue value) {
    instance.overrides[property_id] = std::move(value);
}

bool has_override(const ComponentInstance& instance, const PropertyID property_id) {
    return instance.overrides.contains(property_id);
}

MaybeRef<const PropertyValue> get_override(const ComponentInstance& instance, const PropertyID property_id) {
    auto it = instance.overrides.find(property_id);
    if (it != instance.overrides.end()) {
        return std::cref(it->second);
    }
    return {};
}

void clear_override(ComponentInstance& instance, const PropertyID property_id) {
    instance.overrides.erase(property_id);
}
