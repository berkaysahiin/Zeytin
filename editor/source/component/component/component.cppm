export module zeytin.component;

import zeytin.property;
import zeytin.annotation;
import zeytin.utility.typedefs;

export {
    using ComponentID = uint64;

    struct Component {
        ComponentID id;
        String type;
        List<Property> properties;
		HashMap<PropertyID, AnnotationMap> annotations;
    };

    using PropertyPredicate = Function<bool(ConstRef<Property>)>;

    MaybeRef<Property> get_property(Ref<Component> component, StringView name);
    MaybeRef<Property> get_property(Ref<Component> component, PropertyPredicate pred);

    MaybeRef<const Property> get_property(ConstRef<Component> component, StringView name);
    MaybeRef<const Property> get_property(ConstRef<Component> component, PropertyPredicate pred);
    
    void set_property(Ref<Component> component, StringView name, PropertyValue value);
    bool is_valid(ConstRef<Component> component);
    
    MaybeRef<const AnnotationValue> get_annotation(ConstRef<Component> component, ConstRef<Property> prop, const AnnotationKey key);
    bool has_annotation(ConstRef<Component> component, ConstRef<Property> prop, AnnotationKey key);
    
    AnnotationKey annotation_key_from_string(StringView key);
    StringView annotation_key_to_string(AnnotationKey key);
}
