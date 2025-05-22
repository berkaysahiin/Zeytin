#include "action/action_deserializer.h"

#include <cstdio>
#include <string>
#include <cassert>
#include <memory>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>     
#include <rttr/type>

#include "action/iaction.h"

namespace rttr_json {

namespace {

rttr::variant extract_basic_types(const rapidjson::Value& json_value)
{
    switch(json_value.GetType())
    {
        case rapidjson::kStringType:
            return std::string(json_value.GetString());
        case rapidjson::kFalseType:
        case rapidjson::kTrueType:
            return json_value.GetBool();
        case rapidjson::kNumberType:
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
        case rapidjson::kNullType:
        case rapidjson::kObjectType:
        case rapidjson::kArrayType: 
            break;
    }
    return rttr::variant();
}

void set_action_inputs(rttr::instance action_obj, const rapidjson::Value& inputs)
{
    if (!inputs.IsObject()) return;

    const auto properties = action_obj.get_derived_type().get_properties();

    for (auto prop : properties) {
        auto member_it = inputs.FindMember(prop.get_name().data());
        if (member_it == inputs.MemberEnd()) {
            continue;
        }

        rttr::variant extracted_value = extract_basic_types(member_it->value);
        if (extracted_value.is_valid() && extracted_value.convert(prop.get_type())) {
            prop.set_value(action_obj, extracted_value);
        }
    }
}

} // anonymous namespace

rttr::variant deserialize_action(const std::string& action_json) {
    rapidjson::Document document;
    document.Parse(action_json.c_str());
    
    assert(!document.HasParseError());
    assert(document.HasMember("type") && document.HasMember("type"));

    const std::string& action_type = document["type"].GetString();
    rttr::type rttr_type = rttr::type::get_by_name(action_type);
    
    assert(rttr_type.is_valid());

    rttr::variant action_variant = rttr_type.create();
    
    assert(action_variant.is_valid());

    if (document.HasMember("inputs") && document["inputs"].IsObject()) {
        set_action_inputs(action_variant, document["inputs"]);
    }

    return action_variant;
}

} 
