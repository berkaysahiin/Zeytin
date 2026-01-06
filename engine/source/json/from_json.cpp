module;

#include <cstdio>
#include <string>
#include <cassert>
#include <filesystem>
#include <fstream>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h> 
#include <rapidjson/document.h>     
#include <rttr/type>

#include "debugging/assert.h"

module zeytin.json;
import zeytin.entity;
import zeytin.component;
import zeytin.logger;

using namespace rapidjson;
using namespace rttr;

namespace
{

void fromjson_recursively(instance obj, Value& json_object);


variant extract_basic_types(Value& json_value)
{
    switch(json_value.GetType())
    {
        case kStringType:
        {
            return std::string(json_value.GetString());
            break;
        }
        case kNullType:     break;
        case kFalseType:
        case kTrueType:
        {
            return json_value.GetBool();
            break;
        }
        case kNumberType:
        {
            if (json_value.IsInt())
                return json_value.GetInt();
            else if (json_value.IsDouble())
                return json_value.GetDouble();
            else if (json_value.IsUint())
                return json_value.GetUint();
            else if (json_value.IsInt64())
                return json_value.GetInt64();
            else if (json_value.IsUint64())
                return json_value.GetUint64();
            break;
        }
        // we handle only the basic types here
        case kObjectType:
        case kArrayType: return variant();
    }

    return variant();
}


static void write_array_recursively(variant_sequential_view& view, Value& json_array_value)
{
    view.set_size(json_array_value.Size());
    const type array_value_type = view.get_rank_type(1);

    for (SizeType i = 0; i < json_array_value.Size(); ++i)
    {
        auto& json_index_value = json_array_value[i];
        if (json_index_value.IsArray())
        {
            auto sub_array_view = view.get_value(i).create_sequential_view();
            write_array_recursively(sub_array_view, json_index_value);
        }
        else if (json_index_value.IsObject())
        {
            variant var_tmp = view.get_value(i);
            variant wrapped_var = var_tmp.extract_wrapped_value();
            fromjson_recursively(wrapped_var, json_index_value);
            view.set_value(i, wrapped_var);
        }
        else
        {
            variant extracted_value = extract_basic_types(json_index_value);
            if (extracted_value.convert(array_value_type))
                view.set_value(i, extracted_value);
        }
    }
}

variant extract_value(Value::MemberIterator& itr, const type& t)
{
    auto& json_value = itr->value;
    variant extracted_value = extract_basic_types(json_value);
    const bool could_convert = extracted_value.convert(t);
    if (!could_convert)
    {
        if (json_value.IsObject())
        {
            constructor ctor = t.get_constructor();
            for (auto& item : t.get_constructors())
            {
                if (item.get_instantiated_type() == t)
                    ctor = item;
            }
            extracted_value = ctor.invoke();
            fromjson_recursively(extracted_value, json_value);
        }
    }

    return extracted_value;
}

static void write_associative_view_recursively(variant_associative_view& view, Value& json_array_value)
{
    for (SizeType i = 0; i < json_array_value.Size(); ++i)
    {
        auto& json_index_value = json_array_value[i];
        if (json_index_value.IsObject()) // a key-value associative view
        {
            Value::MemberIterator key_itr = json_index_value.FindMember("key");
            Value::MemberIterator value_itr = json_index_value.FindMember("value");

            if (key_itr != json_index_value.MemberEnd() &&
                value_itr != json_index_value.MemberEnd())
            {
                auto key_var = extract_value(key_itr, view.get_key_type());
                auto value_var = extract_value(value_itr, view.get_value_type());
                if (key_var && value_var)
                {
                    view.insert(key_var, value_var);
                }
            }
        }
        else // a key-only associative view
        {
            variant extracted_value = extract_basic_types(json_index_value);
            if (extracted_value && extracted_value.convert(view.get_key_type()))
                view.insert(extracted_value);
        }
    }
}

void fromjson_recursively(instance obj2, Value& json_object)
{
    assert(obj2.is_valid());
    assert(json_object.IsObject());

    instance obj = obj2.get_type().get_raw_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;
    assert(obj.is_valid());

    const auto prop_list = obj.get_derived_type().get_properties();

    for (auto prop : prop_list)
    {
        assert(prop.is_valid());

        Value::MemberIterator ret = json_object.FindMember(prop.get_name().data());
        if (ret == json_object.MemberEnd()) {
            continue;
        }

        const type value_t = prop.get_type();

        auto& json_value = ret->value;
        switch(json_value.GetType())
        {
            case kArrayType:
            {
                variant var;
                if (value_t.is_sequential_container())
                {
                    var = prop.get_value(obj);
                    auto view = var.create_sequential_view();
                    write_array_recursively(view, json_value);
                }
                else if (value_t.is_associative_container())
                {
                    var = prop.get_value(obj);
                    auto associative_view = var.create_associative_view();
                    write_associative_view_recursively(associative_view, json_value);
                }

                prop.set_value(obj, var);
                break;
            }
            case kObjectType:
            {
                variant var = prop.get_value(obj);
                fromjson_recursively(var, json_value);
                prop.set_value(obj, var);
                break;
            }
            default:
            {
                variant extracted_value = extract_basic_types(json_value);
                if (extracted_value.convert(value_t))  {
                    prop.set_value(obj, extracted_value);
                }

            }
        }
    }
}


const std::string value_as_string(const rapidjson::Value& value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

bool from_internal(const std::string& json, rttr::instance obj)
{
    Document document;  
    document.Parse(json.c_str());
    assert(!document.HasParseError());

    fromjson_recursively(obj, document);

    return true;
}


rttr::variant from(EntityID entity_id, const std::string& json)
{
    Document document;
    document.Parse(json.c_str());
    assert(!document.HasParseError());
    assert(document.HasMember("type"));
    assert(document.HasMember("value"));

    Value& type = document["type"];
    Value& value = document["value"];

    rttr::type rttr_type = rttr::type::get_by_name(type.GetString());
	std::string type_name = type.GetString();
	ASSERT(rttr_type.is_valid(), "Cannot find type \"{}\". Make sure it is registered.", type_name);

    rttr::variant obj = rttr_type.create();
    assert(obj.is_valid()); // component type must have default constructor
	rttr_type.set_property_value("entity_id", obj, entity_id);

    from_internal(value_as_string(value), obj);

    return obj;
}

rttr::variant from(EntityID entity_id, const std::filesystem::path& json_path)
{
    std::ifstream file(json_path);
    assert(file.is_open());

    std::string json_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();

    return from(entity_id, json_content);
}

}  // end of anonynmous namepsace

namespace rttr_json {

EntityID deserialize_entity(const std::string& entity_json, EntityID& entity, std::vector<rttr::variant>& variants) {
    Document document;
    document.Parse(entity_json.c_str());
    assert(!document.HasParseError());

    assert(document.HasMember("entity_id") && document["entity_id"].GetUint64());
    auto entity_id = document["entity_id"].GetUint64();
    entity = entity_id;

    assert(document.HasMember("variants") && document["variants"].IsArray());
    const rapidjson::Value& variant_values = document["variants"];

    for(rapidjson::SizeType i = 0; i < variant_values.Size(); ++i) {
        const rapidjson::Value& variant = variant_values[i];

        const std::string& input_type = variant["type"].GetString();
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        variant.Accept(writer);
        const std::string variant_str = buffer.GetString();

        rttr::variant var = from(entity_id, variant_str);
        var.get_type().set_property_value("entity_id", var, entity_id);
        variants.push_back(std::move(var));
    }
    return entity_id;
}

rttr::variant deserialize_component(EntityID entity_id, const std::string& component_json) {
    return from(entity_id, component_json);
}

}   // end of namespace
