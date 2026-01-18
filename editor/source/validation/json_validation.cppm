module;

#include "rapidjson/document.h"

export module zeytin.validation.json;

export namespace json_validation {

    bool has_string_member(const rapidjson::Value& obj, const char* key) {
        return obj.IsObject() && obj.HasMember(key) && obj[key].IsString();
    }

    bool has_object_member(const rapidjson::Value& obj, const char* key) {
        return obj.IsObject() && obj.HasMember(key) && obj[key].IsObject();
    }

    bool has_array_member(const rapidjson::Value& obj, const char* key) {
        return obj.IsObject() && obj.HasMember(key) && obj[key].IsArray();
    }

    bool has_uint64_member(const rapidjson::Value& obj, const char* key) {
        return obj.IsObject() && obj.HasMember(key) && obj[key].IsUint64();
    }
}
