module;

#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_set>

#define RAPIDJSON_HAS_STDSTRING 1
#include "rapidjson/prettywriter.h"
#include <rapidjson/document.h>     
#include <rttr/type.h>

module zeytin.json;
import zeytin.resource;

using namespace rapidjson;
using namespace rttr;

namespace
{

void to_json_recursively(const instance& obj, PrettyWriter<StringBuffer>& writer);

bool write_variant(const variant& var, PrettyWriter<StringBuffer>& writer);

bool write_atomic_types_to_json(const type& t, const variant& var, PrettyWriter<StringBuffer>& writer)
{
    if (t.is_arithmetic())
    {
        if (t == type::get<bool>())
            writer.Bool(var.to_bool());
        else if (t == type::get<char>())
            writer.Bool(var.to_bool());
        else if (t == type::get<int8_t>())
            writer.Int(var.to_int8());
        else if (t == type::get<int16_t>())
            writer.Int(var.to_int16());
        else if (t == type::get<int32_t>())
            writer.Int(var.to_int32());
        else if (t == type::get<int64_t>())
            writer.Int64(var.to_int64());
        else if (t == type::get<uint8_t>())
            writer.Uint(var.to_uint8());
        else if (t == type::get<uint16_t>())
            writer.Uint(var.to_uint16());
        else if (t == type::get<uint32_t>())
            writer.Uint(var.to_uint32());
        else if (t == type::get<uint64_t>())
            writer.Uint64(var.to_uint64());
        else if (t == type::get<float>())
            writer.Double(var.to_double());
        else if (t == type::get<double>())
            writer.Double(var.to_double());

        return true;
    }
    else if (t.is_enumeration())
    {
        bool ok = false;
        auto result = var.to_string(&ok);
        if (ok)
        {
            writer.String(var.to_string());
        }
        else
        {
            ok = false;
            auto value = var.to_uint64(&ok);
            if (ok)
                writer.Uint64(value);
            else
                writer.Null();
        }

        return true;
    }
    else if (t == type::get<std::string>())
    {
        if (var.can_convert<std::string>()) {
            writer.String(var.to_string());
            return true;
        }
        
        writer.Null();
        std::cerr << "Failed to convert property to string" << std::endl;
        return false;
    }

    return false;
}


static void write_array(const variant_sequential_view& view, PrettyWriter<StringBuffer>& writer)
{
    writer.StartArray();
    
    try {
        for (const auto& item : view)
        {
            if (item.is_sequential_container())
            {
                write_array(item.create_sequential_view(), writer);
            }
            else
            {
                variant wrapped_var = item.extract_wrapped_value();
                type value_type = wrapped_var.get_type();
                if (value_type.is_arithmetic() || value_type == type::get<std::string>() || value_type.is_enumeration())
                {
                    write_atomic_types_to_json(value_type, wrapped_var, writer);
                }
                else // object
                {
                    to_json_recursively(wrapped_var, writer);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in write_array: " << e.what() << std::endl;
    }
    
    writer.EndArray();
}


static void write_associative_container(const variant_associative_view& view, PrettyWriter<StringBuffer>& writer)
{
    static const string_view key_name("key");
    static const string_view value_name("value");

    writer.StartArray();

    try {
        if (view.is_key_only_type())
        {
            for (auto& item : view)
            {
                write_variant(item.first, writer);
            }
        }
        else
        {
            for (auto& item : view)
            {
                writer.StartObject();
                writer.String(key_name.data(), static_cast<rapidjson::SizeType>(key_name.length()), false);

                write_variant(item.first, writer);

                writer.String(value_name.data(), static_cast<rapidjson::SizeType>(value_name.length()), false);

                write_variant(item.second, writer);

                writer.EndObject();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in write_associative_container: " << e.what() << std::endl;
    }

    writer.EndArray();
}

bool write_variant(const variant& var, PrettyWriter<StringBuffer>& writer)
{
    if (!var.is_valid()) {
        writer.Null();
        return false;
    }
    
    auto value_type = var.get_type();
    auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
    bool is_wrapper = wrapped_type != value_type;

    if (write_atomic_types_to_json(is_wrapper ? wrapped_type : value_type,
                                   is_wrapper ? var.extract_wrapped_value() : var, writer))
    {
        return true;
    }
    else if (var.is_sequential_container())
    {
        write_array(var.create_sequential_view(), writer);
        return true;
    }
    else if (var.is_associative_container())
    {
        write_associative_container(var.create_associative_view(), writer);
        return true;
    }
    else
    {
        auto child_props = is_wrapper ? wrapped_type.get_properties() : value_type.get_properties();
        if (!child_props.empty())
        {
            to_json_recursively(var, writer);
            return true;
        }
        else
        {
            try {
                bool ok = false;
                auto text = var.to_string(&ok);
                if (!ok)
                {
                    writer.String(text);
                    return false;
                }

                writer.String(text);
                return true;
            } catch (const std::exception& e) {
                std::cerr << "Exception converting variant to string: " << e.what() << std::endl;
                writer.Null();
                return false;
            }
        }
    }
}


void to_json_recursively(const instance& obj2, PrettyWriter<StringBuffer>& writer)
{
    writer.StartObject();

    if (!obj2.is_valid()) {
        std::cerr << "Invalid object instance in to_json_recursively" << std::endl;
        writer.EndObject();
        return;
    }

    instance obj = obj2.get_type().get_raw_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;

    if (!obj.is_valid()) {
        std::cerr << "Invalid wrapped object instance in to_json_recursively" << std::endl;
        writer.EndObject();
        return;
    }

    auto prop_list = obj.get_derived_type().get_properties();
    std::unordered_set<std::string> serialized_properties;
    serialized_properties.reserve(prop_list.size());

    for (auto prop : prop_list)
    {
        if (!prop.is_valid()) {
            std::cerr << "Invalid property encountered" << std::endl;
            continue;
        }
        
        if (prop.get_metadata("NO_SERIALIZE"))
            continue;
 
        try {
            variant prop_value = prop.get_value(obj);
            if (!prop_value.is_valid())
                continue; 
 
            const auto name = prop.get_name();
            std::string name_str = name.to_string();
            if (!serialized_properties.insert(name_str).second) {
                continue;
            }

            writer.String(name_str.c_str(), static_cast<rapidjson::SizeType>(name_str.size()), false);
            if (!write_variant(prop_value, writer))
            {
                std::cerr << "Failed to serialize property: " << name_str << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception handling property: " << e.what() << std::endl;
        }
    }


    writer.EndObject();
}


std::string serialize_value(rttr::instance obj)
{
    if (!obj.is_valid()) {
        std::cerr << "Invalid object passed to serialize_value" << std::endl;
        return std::string();
    }

    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);

    try {
        to_json_recursively(obj, writer);
    } catch (const std::exception& e) {
        std::cerr << "Exception in serialize_value: " << e.what() << std::endl;
        return std::string();
    }

    return sb.GetString();
}

std::string to(rttr::instance obj, const std::string& path) {
    if (!obj.is_valid()) {
        std::cerr << "Invalid object passed to to() function" << std::endl;
        return std::string();
    }
    
    try {
        const std::string serialized_value = serialize_value(obj);
        if (serialized_value.empty()) {
            std::cerr << "Failed to serialize object" << std::endl;
            return std::string();
        }

        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        rttr::type type = obj.get_type().get_raw_type();
        if (!type.is_valid()) {
            std::cerr << "Invalid type for serialization" << std::endl;
            return std::string();
        }
        
        document.AddMember("type", type.get_name().to_string(), allocator);

        rapidjson::Document value_doc;

        const auto& parse_result = value_doc.Parse(serialized_value.c_str());
        if (parse_result.HasParseError()) {
            std::cerr << "JSON parse error at offset " << parse_result.GetErrorOffset() << std::endl;
            return std::string();
        }
        
        if (!value_doc.IsObject()) {
            std::cerr << "Serialized value is not a valid JSON object" << std::endl;
            return std::string();
        }

        document.AddMember("value", value_doc, allocator);

        std::filesystem::create_directories(std::filesystem::path(path).parent_path());
        
        std::ofstream out(path);
        if (!out.is_open()) {
            std::cerr << "Failed to open output file: " << path << std::endl;
            return std::string();
        }
        
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

        document.Accept(writer);
        
        auto json_string = buffer.GetString();
        out << json_string;
        
        if (out.fail()) {
            std::cerr << "Failed to write to file: " << path << std::endl;
        }
        
        out.close();

        return json_string;
    } catch (const std::exception& e) {
        std::cerr << "Exception in to() function: " << e.what() << std::endl;
        return std::string();
    }
}

}  // end of anonymous namespace

namespace rttr_json  {

std::string serialize_entity(const EntityID EntityID, const std::vector<rttr::variant>& variants) {
    if (variants.empty()) {
        std::cerr << "Serializing entity with no variants" << std::endl;
    }
    
    try {
        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        document.AddMember("entity_id", EntityID, allocator);

        rapidjson::Value variants_array(rapidjson::kArrayType);

        for (const auto& variant : variants) {
            if (!variant.is_valid()) {
                std::cerr << "Invalid variant in serialize_entity" << std::endl;
                continue;
            }
            
            const std::string serialized_variant = serialize_value(variant);
            if (serialized_variant.empty()) {
                std::cerr << "Failed to serialize variant of type: " << variant.get_type().get_name().to_string() << std::endl;
                continue;
            }

            rapidjson::Document variant_document;
            const auto& parse_result = variant_document.Parse(serialized_variant.c_str());
            if (parse_result.HasParseError()) {
                std::cerr << "JSON parse error at offset " << parse_result.GetErrorOffset() << std::endl;
                continue;
            }
            
            if (!variant_document.IsObject()) {
                std::cerr << "Serialized variant is not a valid JSON object" << std::endl;
                continue;
            }

            rapidjson::Value variant_obj(rapidjson::kObjectType);
            variant_obj.AddMember("type", variant.get_type().get_name().to_string(), allocator);

            rapidjson::Value value_obj(rapidjson::kObjectType);
            value_obj.CopyFrom(variant_document, allocator);
            variant_obj.AddMember("value", value_obj, allocator);

            variants_array.PushBack(variant_obj, allocator);
        }

        document.AddMember("variants", variants_array, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);

        return buffer.GetString();
    } catch (const std::exception& e) {
        std::cerr << "Exception in serialize_entity: " << e.what() << std::endl;
        return std::string();
    }
}

std::string serialize_entity(const EntityID EntityID, const std::vector<rttr::variant>& variants, const std::filesystem::path& path) {
    try {
        std::string json_string = serialize_entity(EntityID, variants);
        if (json_string.empty()) {
            std::cerr << "Failed to serialize entity" << std::endl;
            return std::string();
        }
        
        std::filesystem::create_directories(path.parent_path());
        
        std::ofstream out(path);
        if (!out.is_open()) {
            std::cerr << "Failed to open output file: " << path.string() << std::endl;
            return json_string;
        }
        
        out << json_string;
        
        if (out.fail()) {
            std::cerr << "Failed to write to file: " << path.string() << std::endl;
        }
        
        out.close();

        return json_string;
    } catch (const std::exception& e) {
        std::cerr << "Exception in serialize_entity with path: " << e.what() << std::endl;
        return std::string();
    }
}

} // end of namespace
