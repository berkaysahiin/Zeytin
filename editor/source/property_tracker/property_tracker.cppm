module;

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

export module zeytin.property.tracker;

import zeytin.singleton;
import zeytin.command.property;

export class PropertyTracker : public Singleton<PropertyTracker> {
    friend class Singleton<PropertyTracker>;
public:
    ~PropertyTracker();

    void update();
    bool is_tracked(const PropertyLocation& location) const;
    std::size_t get_tracked_count(uint64_t entity_id) const;
    std::vector<std::string> get_tracked_keys(uint64_t entity_id) const;
    void toggle_tracking(const PropertyLocation& location, const std::string& key_type);

    void handle_tracked_value(uint64_t entity_id,
                              const std::string& variant_type,
                              const std::string& key_type,
                              const std::string& key_path,
                              const std::string& value);

private:
    PropertyTracker();

    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
