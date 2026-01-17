module;

#include <string> // IWYU pragma: keep

module zeytin.component;
import zeytin.logger;
import zeytin.resource;
import zeytin.common.guid;

MaybeRef<Property> get_property(Component& component, StringView name) {
    auto it = std::ranges::find_if(component.properties,
        [name](const Property& prop) { return prop.name == name; });
    
    if (it != component.properties.end()) {
        return std::ref(*it);
    }

    return {};
}

MaybeRef<Property> get_property(Component& component, PropertyPredicate pred) {
    auto it = std::ranges::find_if(component.properties, pred);
    
    if (it != component.properties.end()) {
        return std::ref(*it);
    }

    return {};
}

MaybeRef<const Property> get_property(const Component& component, StringView name) {
    auto it = std::ranges::find_if(component.properties,
        [name](const Property& prop) { return prop.name == name; });
    
    if (it != component.properties.end()) {
        return std::cref(*it);
    }
    return {};
}

MaybeRef<const Property> get_property(const Component& component, PropertyPredicate pred) {
    auto it = std::ranges::find_if(component.properties, pred);
    
    if (it != component.properties.end()) {
        return std::cref(*it);
    }

    return {};
}

void set_property(Component& component, StringView name, PropertyValue value) {
    if (auto prop = get_property(component, name)) {
        prop->get().value = value;
    } else {
        Property new_prop;
        new_prop.name = String {name};
        new_prop.value = value;
		new_prop.id = component.properties.empty() ? 0 : component.properties.back().id + 1;
        component.properties.push_back(std::move(new_prop));
    }
}

bool has_property(const Component& component, StringView name) {
    return get_property(component, name).has_value();
}

bool is_valid(const Component& component) {
    if (component.type.empty()) {
        log_warning("Component validation failed: empty type");
        return false;
    }
    
    return true;
}

MaybeRef<const AnnotationValue> get_annotation(const Component& component, const Property& prop, const AnnotationKey key) {
    auto ann_map_it = component.annotations.find(prop.id);
    if (ann_map_it == component.annotations.end()) {
        return {};
    }
    
    auto it = ann_map_it->second.find(key);
    if (it != ann_map_it->second.end()) {
        return std::cref(it->second);
    }

    return {};
}

bool has_annotation(const Component& component, const Property& prop, const AnnotationKey key) {
    auto ann_map_it = component.annotations.find(prop.id);
    if (ann_map_it == component.annotations.end()) {
        return false;
    }
    return ann_map_it->second.contains(key);
}


AnnotationKey annotation_key_from_string(StringView key) {
	using enum AnnotationKey;

	#define CHECK_STR_TO_ENUM(ENUM) \
		if ( key == #ENUM ) { \
	  		return ENUM; \
		}

	CHECK_STR_TO_ENUM(NAME);
	CHECK_STR_TO_ENUM(TOOLTIP);
	CHECK_STR_TO_ENUM(MIN);
	CHECK_STR_TO_ENUM(MAX);
	CHECK_STR_TO_ENUM(GROUP);
	CHECK_STR_TO_ENUM(HIDDEN);
	CHECK_STR_TO_ENUM(SAVE);
	CHECK_STR_TO_ENUM(READONLY);
	CHECK_STR_TO_ENUM(FILEPATH);
	CHECK_STR_TO_ENUM(ENABLE_IF);
	CHECK_STR_TO_ENUM(DEPRECATED);
	CHECK_STR_TO_ENUM(HELPER);

	return UNKNOWN;
}

StringView annotation_key_to_string(const AnnotationKey key) {
	using enum AnnotationKey;

	#define CHECK_ENUM_TO_STR(ENUM) case ENUM: return #ENUM;

    switch (key) {
        CHECK_ENUM_TO_STR(NAME)
        CHECK_ENUM_TO_STR(TOOLTIP)
        CHECK_ENUM_TO_STR(MIN)
        CHECK_ENUM_TO_STR(MAX)
        CHECK_ENUM_TO_STR(GROUP)
        CHECK_ENUM_TO_STR(HIDDEN)
        CHECK_ENUM_TO_STR(SAVE)
        CHECK_ENUM_TO_STR(READONLY)
        CHECK_ENUM_TO_STR(FILEPATH)
        CHECK_ENUM_TO_STR(ENABLE_IF)
        CHECK_ENUM_TO_STR(DEPRECATED)
        CHECK_ENUM_TO_STR(HELPER)
        CHECK_ENUM_TO_STR(UNKNOWN)
    }

    return "UNKNOWN";
}
