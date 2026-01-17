export module zeytin.component.instance;

import zeytin.component;
import zeytin.component.document;
import zeytin.property;
import zeytin.utility.typedefs;
import zeytin.common.guid;

export {
    using ComponentInstanceID = uint64;
    using PropertyOverrideMap = HashMap<PropertyID, PropertyValue>;

    struct ComponentInstance {
        ComponentInstanceID id;
        ComponentID component_id;
        ComponentDocumentID component_document_id;
        PropertyOverrideMap overrides;
    };

    ComponentInstance create_component_instance(const ComponentID component_id);

    void set_override(ComponentInstance& instance, const PropertyID property_id, PropertyValue value);
    bool has_override(const ComponentInstance& instance, const PropertyID property_id);
    MaybeRef<const PropertyValue> get_override(const ComponentInstance& instance, const PropertyID property_id);
    void clear_override(ComponentInstance& instance, const PropertyID property_id);
}
