module;

#include <string> // IWYU pragma: keep
#include "rapidjson/document.h"

module zeytin.entity.document.conversion;

import zeytin.component;
import zeytin.component.registry;
import zeytin.component.instance;
import zeytin.entity;
import zeytin.entity.document;
import zeytin.logger;
import zeytin.property;
import zeytin.utility.json;
import zeytin.utility.typedefs;

namespace {
    void instance_set_properties(const rapidjson::Value& value_obj, ComponentInstance& instance);
    void write_value_from_component(const ComponentInstance& instance, rapidjson::Value& value_obj, rapidjson::Document::AllocatorType& allocator);
}

#define JSON_KEY_ENTITY_ID "entity_id"
#define JSON_KEY_COMPONENT_ARRAY "variants"
#define JSON_KEY_COMPONENT_VALUE "value"
#define JSON_KEY_COMPONENT_TYPE "type"

Entity entity_from_document(const EntityDocument& document) {
    Entity entity{};
    entity.id = document.get_id();
    entity.name = document.get_name();

    const rapidjson::Document& doc = document.get_document();
    if (!doc.IsObject()) {
        log_warning("Entity conversion: document '{}' is not an object", entity.name);
        return entity;
    }

    if (!doc.HasMember(JSON_KEY_COMPONENT_ARRAY) || !doc[JSON_KEY_COMPONENT_ARRAY].IsArray()) {
        log_warning("Entity conversion: document '{}' missing variants array", entity.name);
        return entity;
    }

    const auto& variants = doc[JSON_KEY_COMPONENT_ARRAY].GetArray();
    entity.components.reserve(variants.Size());

    for (const auto& variant : variants) {
        if (!variant.IsObject() || !variant.HasMember(JSON_KEY_COMPONENT_TYPE) || !variant[JSON_KEY_COMPONENT_TYPE].IsString()) {
            log_warning("Entity conversion: variant missing type in '{}'", entity.name);
            continue;
        }

        const String type = {variant[JSON_KEY_COMPONENT_TYPE].GetString(), variant[JSON_KEY_COMPONENT_TYPE].GetStringLength()};
        const MaybeRef<Component> component_opt = get_component_by_name(type);
        if (!component_opt) {
            log_warning("Entity conversion: component '{}' missing for entity '{}'", type, entity.name);
            continue;
        }

        const MaybeRef<ComponentInstance> instance_opt = create_instance(component_opt->get());
        if (!instance_opt) {
            log_warning("Entity conversion: failed to create instance for '{}'", type);
            continue;
        }

        if (variant.HasMember(JSON_KEY_COMPONENT_VALUE) && variant[JSON_KEY_COMPONENT_VALUE].IsObject()) {
            instance_set_properties(variant[JSON_KEY_COMPONENT_VALUE], instance_opt->get());
        }

        entity.components.push_back(instance_opt->get().id);
    }

    return entity;
}

// ==========================================================================================================================================

void document_from_entity(const Entity& entity, EntityDocument& document) {
    rapidjson::Document new_doc;
    new_doc.SetObject();
    auto& allocator = new_doc.GetAllocator();

    new_doc.AddMember(JSON_KEY_ENTITY_ID, static_cast<uint64_t>(entity.id), allocator);

    rapidjson::Value variants_array(rapidjson::kArrayType);

    for (const auto instance_id : entity.components) {
        auto instance_opt = get_instance_const(instance_id);
        if (!instance_opt) {
            log_warning("Entity conversion: missing instance {} for entity '{}'", instance_id, entity.name);
            continue;
        }

        const auto& instance = instance_opt->get();
        auto component_opt = get_component_const(instance.component_id);
        if (!component_opt) {
            log_warning("Entity conversion: missing component {} for entity '{}'", instance.component_id, entity.name);
            continue;
        }

        const auto& component = component_opt->get();

        rapidjson::Value variant_obj(rapidjson::kObjectType);

        rapidjson::Value type_value;
        type_value.SetString(component.type.c_str(), static_cast<rapidjson::SizeType>(component.type.size()), allocator);
        variant_obj.AddMember(JSON_KEY_COMPONENT_TYPE, type_value, allocator);

        rapidjson::Value value_obj(rapidjson::kObjectType);
        write_value_from_component(instance, value_obj, allocator);
        value_obj.AddMember(JSON_KEY_ENTITY_ID, static_cast<uint64_t>(entity.id), allocator);

        variant_obj.AddMember(JSON_KEY_COMPONENT_VALUE, value_obj, allocator);
        variants_array.PushBack(variant_obj, allocator);
    }

    new_doc.AddMember(JSON_KEY_COMPONENT_ARRAY, variants_array, allocator);

    document.set_document(std::move(new_doc));
}

namespace {
    void instance_set_properties(const rapidjson::Value& value_obj, ComponentInstance& instance) {
        if (!value_obj.IsObject()) {
            return;
        }

        for (const auto& member : value_obj.GetObject()) {
            PropertyValue parsed_value = json_value_from_rapidjson(member.value);
            StringView prop_name = {member.name.GetString(), member.name.GetStringLength()};

            (void)instance_set_property(instance, prop_name, parsed_value);
        }
    }

    void write_value_from_component(const ComponentInstance& instance, rapidjson::Value& value_obj, rapidjson::Document::AllocatorType& allocator) {
        const List<Property> properties = instance_get_properties(instance);
        for (const Property& prop : properties) {
            rapidjson::Value key;
            key.SetString(prop.name.c_str(), static_cast<rapidjson::SizeType>(prop.name.size()), allocator);
            value_obj.AddMember(key, rapidjson_value_from_json(prop.value, allocator), allocator);
        }
    }
}
