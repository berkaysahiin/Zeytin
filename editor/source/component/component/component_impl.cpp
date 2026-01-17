module;

#include <rapidjson/document.h>
#include <variant>
#include <fstream>

module zeytin.component;
import zeytin.logger;
import zeytin.resource;
import zeytin.common.guid;

namespace {
    JsonValue get_json_value(const rapidjson::Value& value) {
        if (value.IsNull()) {
            return std::monostate{};
        }
        if (value.IsBool()) {
            return value.GetBool();
        }
        if (value.IsInt64()) {
            return value.GetInt64();
        }
        if (value.IsDouble()) {
            return value.GetDouble();
        }
        if (value.IsString()) {
            return std::string{value.GetString()};
        }
        return std::monostate{};
    }
}


Maybe<Component> parse_component_file(PathView file_path) {
    Component component;
    component.id = generate_unique_id();
    component.source_file = file_path;
    
    std::ifstream in_file(file_path);
    if (!in_file.is_open()) {
        log_error("Failed to open component file: {}", file_path.string());
        return {};
    }
    
    const String json_string{
        std::istreambuf_iterator<char>(in_file),
        std::istreambuf_iterator<char>()
    };

    rapidjson::Document document;
    document.Parse(json_string.c_str());
    
    if (document.HasParseError()) {
        log_error("JSON parse error in component file: {} at offset: {}", file_path.string(), document.GetErrorOffset());
        return {};
    }
    
    if (!document.IsObject()) {
        log_error("Component file does not contain a valid JSON object: {}", file_path.string());
        return {};
    }
    
    if (!document.HasMember("type") || !document["type"].IsString()) {
        log_error("Component file missing valid 'type' field: {}", file_path.string());
        return {};
    }
    
    component.type = document["type"].GetString();
    
    if (!document.HasMember("value") || !document["value"].IsObject()) {
        log_error("Component file missing valid 'value' object: {}", file_path.string());
        return {};
    }
    
    const auto& value_object = document["value"];
    component.properties.reserve(value_object.MemberCount());

    PropertyID next_id = 0;
    for (const auto& [name, value] : value_object.GetObject()) {
        Property prop;
        prop.name = String {name.GetString(), name.GetStringLength()};
        prop.value = get_json_value(value);
        prop.id = next_id++;
        component.properties.push_back(std::move(prop));
    }
    
    if (document.HasMember("annotations") && document["annotations"].IsObject()) {
        const auto& annotations_object = document["annotations"];

        for (const auto& [prop_name, annotations_obj] : annotations_object.GetObject()) {
            const StringView property_name {prop_name.GetString(), prop_name.GetStringLength()};
            
			auto prop = find_property(component, [property_name](const Property& p) { return p.name == property_name; });

            if (prop) {
                AnnotationMap property_annotations;
                for (const auto& [key, value] : annotations_obj.GetObject()) {
                    const StringView annotation_key_str {key.GetString(), key.GetStringLength()};
                    const AnnotationKey annotation_key = annotation_key_from_string(annotation_key_str);
                    AnnotationValue annotation_value = get_json_value(value);
                    property_annotations[annotation_key] = std::move(annotation_value);
                }
                component.annotations[prop->get().id] = std::move(property_annotations);
            }
        }
    }

	return component;
}

MaybeRef<Property> get_property(Component& component, StringView name) {
    auto it = std::ranges::find_if(component.properties,
        [name](const Property& prop) { return prop.name == name; });
    
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

MaybeRef<Property> find_property(Component& component, PropertyPredicate pred) {
    auto it = std::ranges::find_if(component.properties, pred);
    
    if (it != component.properties.end()) {
        return std::ref(*it);
    }

    return {};
}

MaybeRef<const Property> find_property(const Component& component, PropertyPredicate pred) {
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
    
    if (component.source_file.empty()) {
        log_warning("Component validation failed: empty source file");
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
