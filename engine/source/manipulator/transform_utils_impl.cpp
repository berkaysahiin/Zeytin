module;

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <cstdint>
#include <string>
#include <vector>

module zeytin.manipulator.transform_utils;
import zeytin.logger;

#ifdef EDITOR_MODE
import zeytin.editor.event;
#endif

using EntityID = uint64_t;

namespace transform_utils {

void send_property_change_command(
    EntityID entity_id,
    const char* property_name,
    float old_value,
    float new_value
) {
#ifdef EDITOR_MODE
    rapidjson::Document msg;
    msg.SetObject();
    auto& alloc = msg.GetAllocator();

    msg.AddMember("type", "property_change_command", alloc);
    msg.AddMember("entity_id", entity_id, alloc);
    msg.AddMember("variant_type", "CTransform", alloc);

    rapidjson::Value prop_name(property_name, alloc);
    msg.AddMember("key_path", prop_name, alloc);
    msg.AddMember("old_value", old_value, alloc);
    msg.AddMember("new_value", new_value, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    EditorEventBus::get().publish<std::string>(EditorEvent::SyncEditor, buffer.GetString());

    log_info("Sent property change command: {} = {} -> {}", property_name, old_value, new_value);
#endif
}

void send_batch_property_change_command(
    EntityID entity_id,
    const std::vector<PropertyChange>& changes
) {
#ifdef EDITOR_MODE
    if (changes.empty()) {
        return;
    }

    rapidjson::Document msg;
    msg.SetObject();
    auto& alloc = msg.GetAllocator();

    msg.AddMember("type", "batch_property_change_command", alloc);
    msg.AddMember("entity_id", entity_id, alloc);
    msg.AddMember("variant_type", "CTransform", alloc);

    // create changes array
    rapidjson::Value changes_array(rapidjson::kArrayType);

    for (const auto& prop_change : changes) {
        rapidjson::Value change(rapidjson::kObjectType);
        rapidjson::Value key_path(prop_change.key_path.c_str(), alloc);
        change.AddMember("key_path", key_path, alloc);
        change.AddMember("old_value", prop_change.old_value, alloc);
        change.AddMember("new_value", prop_change.new_value, alloc);
        changes_array.PushBack(change, alloc);
    }

    msg.AddMember("changes", changes_array, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    EditorEventBus::get().publish<std::string>(EditorEvent::SyncEditor, buffer.GetString());

    log_info("Sent batch property change command with {} changes", changes.size());
#endif
}

}
