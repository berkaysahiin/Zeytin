module;

#include "rapidjson/pointer.h"

#include <unordered_map>
#include <string>
#include <optional>
#include <algorithm>
#include <variant>
#include <chrono>
#include <cstddef>

module zeytin.property.tracker;

import zeytin.selection;
import zeytin.engine.message;
import zeytin.common.message.editor_to_engine.tracked_property_request;
import zeytin.entity.registry;
import zeytin.entity.list;
import zeytin.entity.document;
import zeytin.logger;

namespace {
    std::string make_property_key(const std::string& variant_type, const std::string& key_path) {
        return variant_type + ":" + key_path;
    }

    std::optional<PropertyValue> parse_property_value(const std::string& key_type, const std::string& value) {
        try {
            if (key_type == "int") {
                return PropertyValue{std::stoi(value)};
            }
            if (key_type == "float") {
                return PropertyValue{std::stof(value)};
            }
            if (key_type == "bool") {
                const bool parsed = (value == "true" || value == "1");
                return PropertyValue{parsed};
            }
            if (key_type == "string") {
                return PropertyValue{value};
            }
        } catch (...) {
            return std::nullopt;
        }

        return std::nullopt;
    }
}

struct PropertyTracker::Impl {
    struct TrackedProperty {
        std::string variant_type;
        std::string key_path;
        std::string key_type;
    };

    std::unordered_map<uint64_t, std::unordered_map<std::string, TrackedProperty>> tracked_properties;
    std::chrono::steady_clock::time_point last_request_time{};
    std::chrono::milliseconds request_interval{100};

    bool apply_value(uint64_t entity_id,
                     const std::string& variant_type,
                     const std::string& key_path,
                     const PropertyValue& value) {
        auto entity_list_opt = EntityRegistry::get().get_entity_list();
        if (!entity_list_opt) {
            log_error("PropertyTracker: EntityList not registered in EntityRegistry");
            return false;
        }

        EntityList& entity_list = entity_list_opt->get();
        auto entity_opt = entity_list.find_entity_by_id(entity_id);
        if (!entity_opt) {
            log_error("PropertyTracker: Failed to find entity with ID {}", entity_id);
            return false;
        }

        EntityDocument& entity = entity_opt->get();
        if (!entity.is_valid()) {
            log_error("PropertyTracker: Entity {} is invalid", entity_id);
            return false;
        }

        rapidjson::Document& doc = entity.get_document();
        rapidjson::Value& variants = doc["variants"];

        for (auto& variant : variants.GetArray()) {
            if (!variant.HasMember("type")) {
                continue;
            }

            std::string type = variant["type"].GetString();
            if (type != variant_type) {
                continue;
            }

            std::string pointer_path = "/value/" + key_path;
            std::replace(pointer_path.begin(), pointer_path.end(), '.', '/');

            rapidjson::Pointer pointer(pointer_path.c_str());
            rapidjson::Value* target = pointer.Get(variant);

            if (!target) {
                log_error("PropertyTracker: Property '{}' not found on entity {}", key_path, entity_id);
                return false;
            }

            std::visit([&](auto&& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, int>) {
                    target->SetInt(val);
                } else if constexpr (std::is_same_v<T, float>) {
                    target->SetFloat(val);
                } else if constexpr (std::is_same_v<T, bool>) {
                    target->SetBool(val);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    target->SetString(val.c_str(), doc.GetAllocator());
                }
            }, value);

            return true;
        }

        log_error("PropertyTracker: Variant '{}' not found on entity {}", variant_type, entity_id);
        return false;
    }
};

PropertyTracker::PropertyTracker()
    : pImpl(std::make_unique<Impl>()) {
}

PropertyTracker::~PropertyTracker() = default;

bool PropertyTracker::is_tracked(const PropertyLocation& location) const {
    auto entity_it = pImpl->tracked_properties.find(location.entity_id);
    if (entity_it == pImpl->tracked_properties.end()) {
        return false;
    }

    const std::string key = make_property_key(location.variant_type, location.key_path);
    return entity_it->second.find(key) != entity_it->second.end();
}

std::size_t PropertyTracker::get_tracked_count(uint64_t entity_id) const {
    auto entity_it = pImpl->tracked_properties.find(entity_id);
    if (entity_it == pImpl->tracked_properties.end()) {
        return 0;
    }

    return entity_it->second.size();
}

std::vector<std::string> PropertyTracker::get_tracked_keys(uint64_t entity_id) const {
    std::vector<std::string> keys;
    auto entity_it = pImpl->tracked_properties.find(entity_id);
    if (entity_it == pImpl->tracked_properties.end()) {
        return keys;
    }

    keys.reserve(entity_it->second.size());
    for (const auto& [key, tracked] : entity_it->second) {
        keys.push_back(key);
    }

    return keys;
}

void PropertyTracker::toggle_tracking(const PropertyLocation& location, const std::string& key_type) {
    const std::string key = make_property_key(location.variant_type, location.key_path);
    auto& tracked = pImpl->tracked_properties[location.entity_id];

    auto it = tracked.find(key);
    if (it != tracked.end()) {
        tracked.erase(it);
        return;
    }

    if (key_type.empty()) {
        log_warning("PropertyTracker: Unknown key type for {}", key);
        return;
    }

    tracked.emplace(key, Impl::TrackedProperty{location.variant_type, location.key_path, key_type});

    auto selected_entity = SelectionManager::get().get_selected_entity();
    if (selected_entity && *selected_entity == location.entity_id) {
        send_message_to_engine<EditorTrackedPropertyRequestMessage>(
            location.entity_id,
            location.variant_type,
            key_type,
            location.key_path
        );
    }
}

void PropertyTracker::update() {
    auto selected_entity = SelectionManager::get().get_selected_entity();
    if (!selected_entity) {
        return;
    }

    auto entity_it = pImpl->tracked_properties.find(*selected_entity);
    if (entity_it == pImpl->tracked_properties.end()) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (pImpl->last_request_time.time_since_epoch().count() != 0) {
        const auto elapsed = now - pImpl->last_request_time;
        if (elapsed < pImpl->request_interval) {
            return;
        }
    }

    pImpl->last_request_time = now;

    for (const auto& [key, tracked] : entity_it->second) {
        send_message_to_engine<EditorTrackedPropertyRequestMessage>(
            *selected_entity,
            tracked.variant_type,
            tracked.key_type,
            tracked.key_path
        );
    }
}

void PropertyTracker::handle_tracked_value(uint64_t entity_id,
                                           const std::string& variant_type,
                                           const std::string& key_type,
                                           const std::string& key_path,
                                           const std::string& value) {
    auto selected_entity = SelectionManager::get().get_selected_entity();
    if (!selected_entity || *selected_entity != entity_id) {
        return;
    }

    const std::string key = make_property_key(variant_type, key_path);
    auto entity_it = pImpl->tracked_properties.find(entity_id);
    if (entity_it == pImpl->tracked_properties.end()) {
        return;
    }

    auto tracked_it = entity_it->second.find(key);
    if (tracked_it == entity_it->second.end()) {
        return;
    }

    const auto parsed_value = parse_property_value(key_type, value);
    if (!parsed_value) {
        log_warning("PropertyTracker: Failed to parse value for {}", key);
        return;
    }

    pImpl->apply_value(entity_id, variant_type, key_path, *parsed_value);
}
