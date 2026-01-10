module;

#include <cstdint>
#include <string>
#include <vector>

module zeytin.manipulator.transform_utils;
import zeytin.common.message.engine_to_editor.property_change_command;
import zeytin.common.message.engine_to_editor.batch_property_change_command;
import zeytin.logger;
import zeytin.editor.message;

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
	send_message_to_editor<PropertyChangeCommandMessage>(entity_id, "CTransform", property_name, old_value, new_value);
    log_trace("Sent property change command: {} = {} -> {}", property_name, old_value, new_value);
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

    std::vector<PropertyChangeEntry> entries;
    entries.reserve(changes.size());

    for (const auto& prop_change : changes) {
        PropertyChangeEntry entry;
        entry.key_path = prop_change.key_path;
        entry.old_value = prop_change.old_value;
        entry.new_value = prop_change.new_value;
        entries.push_back(std::move(entry));
    }

	send_message_to_editor<BatchPropertyChangeCommandMessage>(entity_id, "CTransform", std::move(entries));
    log_trace("Sent batch property change command with {} changes", changes.size());
#endif
}

}
