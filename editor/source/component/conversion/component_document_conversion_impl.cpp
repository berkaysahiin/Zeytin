module;

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

module zeytin.component.document.conversion;

import zeytin.annotation;
import zeytin.common.guid;
import zeytin.component.document;
import zeytin.utility.json;
import zeytin.utility.typedefs;
import zeytin.property;

Component component_from_document(const ComponentDocument& document) {
    Component component;

	// TODO: change this to hash ?
    component.id = generate_unique_id();

    const auto& json = document.get_document();
    if (!json.IsObject()) {
        return component;
    }

    if (json.HasMember("type") && json["type"].IsString()) {
        component.type = json["type"].GetString();
    }

    if (json.HasMember("value") && json["value"].IsObject()) {
        const auto& value_object = json["value"];
        component.properties.reserve(value_object.MemberCount());
        PropertyID next_id = 0;

        for (const auto& [name, value] : value_object.GetObject()) {
            Property prop;
            prop.name = String{name.GetString(), name.GetStringLength()};
            prop.value = json_value_from_rapidjson(value);
            prop.id = next_id++;
            component.properties.push_back(std::move(prop));
        }
    }

    if (json.HasMember("annotations") && json["annotations"].IsObject()) {
        const auto& annotations_object = json["annotations"];
        for (const auto& [prop_name, annotations_obj] : annotations_object.GetObject()) {
            const StringView property_name{prop_name.GetString(), prop_name.GetStringLength()};
            auto prop = get_property(component, [property_name](const Property& p) { return p.name == property_name; });
            if (!prop || !annotations_obj.IsObject()) {
                continue;
            }

            AnnotationMap property_annotations;
            for (const auto& [key, value] : annotations_obj.GetObject()) {
                const StringView annotation_key_str{key.GetString(), key.GetStringLength()};
                const AnnotationKey annotation_key = annotation_key_from_string(annotation_key_str);
                property_annotations[annotation_key] = json_value_from_rapidjson(value);
            }

            component.annotations[prop->get().id] = std::move(property_annotations);
        }
    }

    return component;
}

ComponentDocument document_from_component(const Component& component) {
    rapidjson::Document document;
    document.SetObject();
    auto& allocator = document.GetAllocator();

    rapidjson::Value type_value;
    type_value.SetString(component.type.c_str(), static_cast<rapidjson::SizeType>(component.type.size()), allocator);
    document.AddMember("type", type_value, allocator);

    rapidjson::Value value_object(rapidjson::kObjectType);
    for (const auto& prop : component.properties) {
        rapidjson::Value key;
        key.SetString(prop.name.c_str(), static_cast<rapidjson::SizeType>(prop.name.size()), allocator);
        value_object.AddMember(key, rapidjson_value_from_json(prop.value, allocator), allocator);
    }
    document.AddMember("value", value_object, allocator);

    rapidjson::Value annotations_object(rapidjson::kObjectType);
    for (const auto& [property_id, annotations] : component.annotations) {
        auto prop = get_property(component, [property_id](const Property& p) { return p.id == property_id; });
        if (!prop) {
            continue;
        }

        rapidjson::Value prop_name;
        prop_name.SetString(prop->get().name.c_str(), static_cast<rapidjson::SizeType>(prop->get().name.size()), allocator);

        rapidjson::Value annotation_obj(rapidjson::kObjectType);
        for (const auto& [key, value] : annotations) {
            const StringView key_name = annotation_key_to_string(key);
            rapidjson::Value ann_key;
            ann_key.SetString(key_name.data(), static_cast<rapidjson::SizeType>(key_name.size()), allocator);
            annotation_obj.AddMember(ann_key, rapidjson_value_from_json(value, allocator), allocator);
        }

        annotations_object.AddMember(prop_name, annotation_obj, allocator);
    }

    document.AddMember("annotations", annotations_object, allocator);

    return ComponentDocument{std::move(document), component.type};
}
