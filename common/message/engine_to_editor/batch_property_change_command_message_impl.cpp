module;

#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

module zeytin.common.message.engine_to_editor.batch_property_change_command;

std::string BatchPropertyChangeCommandMessage::get_type() const {
    return "batch_property_change_command";
}

std::string BatchPropertyChangeCommandMessage::as_json() const {
    rapidjson::Document msg;
    msg.SetObject();
    auto& alloc = msg.GetAllocator();

    msg.AddMember("type", rapidjson::Value(get_type().c_str(), alloc), alloc);
    msg.AddMember("entity_id", entity_id, alloc);
    msg.AddMember("variant_type", rapidjson::Value(variant_type.c_str(), alloc), alloc);

    rapidjson::Value changes_array(rapidjson::kArrayType);
    for (const auto& change : changes) {
        rapidjson::Value change_obj(rapidjson::kObjectType);
        change_obj.AddMember("key_path", rapidjson::Value(change.key_path.c_str(), alloc), alloc);
        change_obj.AddMember("old_value", change.old_value, alloc);
        change_obj.AddMember("new_value", change.new_value, alloc);
        changes_array.PushBack(change_obj, alloc);
    }

    msg.AddMember("changes", changes_array, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    return buffer.GetString();
}

bool BatchPropertyChangeCommandMessage::from_json(const rapidjson::Value& message) {
    if (!message.IsObject()) {
        return false;
    }

    if (!message.HasMember("type") || !message["type"].IsString()) {
        return false;
    }

    if (get_type() != message["type"].GetString()) {
        return false;
    }

    if (!message.HasMember("entity_id") || !message["entity_id"].IsUint64()) {
        return false;
    }

    if (!message.HasMember("variant_type") || !message["variant_type"].IsString()) {
        return false;
    }

    if (!message.HasMember("changes") || !message["changes"].IsArray()) {
        return false;
    }

    entity_id = message["entity_id"].GetUint64();
    variant_type = message["variant_type"].GetString();
    changes.clear();

    const auto& changes_array = message["changes"];
    for (rapidjson::SizeType i = 0; i < changes_array.Size(); i++) {
        const auto& change = changes_array[i];
        if (!change.IsObject()) {
            continue;
        }

        if (!change.HasMember("key_path") || !change["key_path"].IsString()) {
            continue;
        }

        if (!change.HasMember("old_value") || !change["old_value"].IsNumber()) {
            continue;
        }

        if (!change.HasMember("new_value") || !change["new_value"].IsNumber()) {
            continue;
        }

        PropertyChangeEntry entry;
        entry.key_path = change["key_path"].GetString();
        entry.old_value = change["old_value"].GetFloat();
        entry.new_value = change["new_value"].GetFloat();
        changes.push_back(std::move(entry));
    }

    return true;
}
