module;

#include <cstdint>
#include <string>
#include <vector>

export module zeytin.manipulator.transform_utils;
import zeytin.manipulator;

using EntityID = uint64_t;

export struct PropertyChange {
    std::string key_path;
    float old_value;
    float new_value;
};

export namespace transform_utils {
    void send_property_change_command(
        EntityID entity_id,
        const char* property_name,
        float old_value,
        float new_value
    );

    void send_batch_property_change_command(
        EntityID entity_id,
        const std::vector<PropertyChange>& changes
    );
}
