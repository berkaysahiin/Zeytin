module;

#include <string>
#include <vector>
#include <cstring>
#include "rapidjson/document.h"

module zeytin.validation.component;
import zeytin.logger;

namespace component_validation {

namespace {
    void detect_differences_recursive(
        rapidjson::Value& editor_obj,
        const rapidjson::Value& engine_obj,
        ComponentDifference& diff,
        std::string& path_buffer,
        size_t prefix_length,
        rapidjson::Document::AllocatorType& allocator)
    {
        if (!editor_obj.IsObject()) {
            return;
        }

        for (auto editor_it = editor_obj.MemberBegin(); editor_it != editor_obj.MemberEnd(); ++editor_it) {
            const char* key = editor_it->name.GetString();
            size_t key_len = editor_it->name.GetStringLength();
            
            if (key_len == 4 && std::memcmp(key, "type", 4) == 0) continue;
            if (key_len == 9 && std::memcmp(key, "entity_id", 9) == 0) continue;
            if (key_len == 5 && std::memcmp(key, "value", 5) == 0) continue;
            if (key_len == 11 && std::memcmp(key, "annotations", 11) == 0) continue;

            size_t old_size = path_buffer.size();
            if (prefix_length > 0) {
                path_buffer.push_back('.');
            }
            path_buffer.append(key, key_len);

            bool found_in_engine = engine_obj.IsObject() && engine_obj.HasMember(key);

            if (!found_in_engine) {
                const char* property_name = (prefix_length == 0) ? key : key;
                
                rapidjson::Value value_copy;
                value_copy.CopyFrom(editor_it->value, allocator);
                
                diff.removed_properties.push_back(RemovedProperty(
                    std::string(property_name, key_len),
                    path_buffer,
                    std::move(value_copy)
                ));
                log_info("Detected removed property in {}: {}", diff.component_type, path_buffer);
            } else if (editor_it->value.IsObject()) {
                const auto& engine_value = engine_obj[key];
                if (engine_value.IsObject()) {
                    detect_differences_recursive(
                        editor_it->value,
                        engine_value,
                        diff,
                        path_buffer,
                        path_buffer.size(),
                        allocator
                    );
                }
            }

            path_buffer.resize(old_size);
        }

        for (auto engine_it = engine_obj.MemberBegin(); engine_it != engine_obj.MemberEnd(); ++engine_it) {
            const char* key = engine_it->name.GetString();
            size_t key_len = engine_it->name.GetStringLength();
            
            if (key_len == 4 && std::memcmp(key, "type", 4) == 0) continue;
            if (key_len == 9 && std::memcmp(key, "entity_id", 9) == 0) continue;
            if (key_len == 5 && std::memcmp(key, "value", 5) == 0) continue;
            if (key_len == 11 && std::memcmp(key, "annotations", 11) == 0) continue;

            if (!editor_obj.HasMember(key)) {
                size_t old_size = path_buffer.size();
                if (prefix_length > 0) {
                    path_buffer.push_back('.');
                }
                path_buffer.append(key, key_len);
                
                diff.added_properties.push_back(path_buffer);
                
                path_buffer.resize(old_size);
            }
        }
    }
}

ComponentDifference detect_component_differences(
    const std::string& component_type,
    rapidjson::Value& editor_component,
    const rapidjson::Value& engine_component,
    rapidjson::Document::AllocatorType& allocator)
{
    ComponentDifference diff;
    diff.component_type = component_type;

    if (editor_component.HasMember("value") && editor_component["value"].IsObject() &&
        engine_component.HasMember("value") && engine_component["value"].IsObject()) {
        
        std::string path_buffer;
        path_buffer.reserve(128);
        
        detect_differences_recursive(
            editor_component["value"],
            engine_component["value"],
            diff,
            path_buffer,
            0,
            allocator
        );
    }

    return diff;
}
}
