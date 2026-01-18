module;

#include <utility>
#include <functional>

module zeytin.component.instance;

import zeytin.component.registry;
import zeytin.logger;
import zeytin.property;

static MaybeRef<const PropertyValue> get_override(const ComponentInstance& instance, const PropertyID property_id);
static void set_override(ComponentInstance& instance, const PropertyID property_id, PropertyValue value);
static void clear_override(ComponentInstance& instance, const PropertyID property_id);

bool instance_set_property(ComponentInstance& instance, StringView property_name, PropertyValue value) {
    const MaybeRef<const Component> component_opt = get_component_const(instance.component_id);
    if (!component_opt) {
        log_warning("Cannot find component for instance {} while trying to set property {}", instance.id, property_name);
        return false;
    }

    if (property_is_null(value)) {
        log_warning("Trying to assign null value to property: {} on {}(instance {})", property_name, component_opt->get().type, instance.id);
        return false;
    }

    const Component& component = component_opt->get();

    const MaybeRef<const Property> prop_opt = get_property(component, property_name);
    if (!prop_opt) {
        log_warning("Cannot find property: {} on {} (instance {})", property_name, component_opt->get().type, instance.id);
        return false;
    }

    const Property& property = prop_opt->get();

    if (property_is_equal(property.value, value)) {
        log_trace("Assigned value is same as the default value for property: {} on {} (instance {})", property_name, component_opt->get().type, instance.id);
        clear_override(instance, property.id);
        return true;
    }

    set_override(instance, property.id, value);
	return true;
}

List<Property> instance_get_properties(const ComponentInstance& instance) {
    const MaybeRef<const Component> component_opt = get_component_const(instance.component_id);
    if (!component_opt) {
        log_warning("Cannot find component for instance {} while reading properties", instance.id);
        return {};
    }

    const Component& component = component_opt->get();
    List<Property> properties = component.properties;

    for (Property& prop : properties) {
        const MaybeRef<const PropertyValue> override_opt = get_override(instance, prop.id);
        if (override_opt) {
            prop.value = override_opt->get();
        }
    }

    return properties;
}

List<Property> instance_get_properties(const ComponentInstanceID instance_id) {
    const MaybeRef<const ComponentInstance> instance_opt = get_instance_const(instance_id);
    if (!instance_opt) {
        log_warning("Cannot find instance {} while reading properties", instance_id);
        return {};
    }

    return instance_get_properties(instance_opt->get());
}

// ========================================================================================================================

static MaybeRef<const PropertyValue> get_override(const ComponentInstance& instance, const PropertyID property_id) {
    auto it = instance.overrides.find(property_id);
    if (it != instance.overrides.end()) {
        return std::cref(it->second);
    }
    return {};
}

static void set_override(ComponentInstance& instance, const PropertyID property_id, PropertyValue value) {
    instance.overrides[property_id] = std::move(value);
}

static void clear_override(ComponentInstance& instance, const PropertyID property_id) {
    instance.overrides.erase(property_id);
}
