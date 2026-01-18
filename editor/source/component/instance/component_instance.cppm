export module zeytin.component.instance;

import zeytin.component;
import zeytin.property;
import zeytin.utility.typedefs;

export {
	using ComponentInstanceID = uint64;
	using PropertyOverrideMap = HashMap<PropertyID, PropertyValue>;

	struct ComponentInstance {
		ComponentInstanceID id;
		ComponentID component_id;
		PropertyOverrideMap overrides;
	};

	[[nodiscard]]
	bool instance_set_property(Ref<ComponentInstance> instance, StringView property_name, PropertyValue value);

	[[nodiscard]] 
	List<Property> instance_get_properties(ConstRef<ComponentInstance> instance);

	[[nodiscard]] 
	List<Property> instance_get_properties(ConstRef<ComponentInstanceID> id);
}
