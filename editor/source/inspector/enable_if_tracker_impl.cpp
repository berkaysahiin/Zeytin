module;

#include <unordered_map>
#include <string>
#include <optional>
#include <chrono>
#include <algorithm>
#include <cctype>

module zeytin.inspector.enable_if;

import zeytin.engine.message;
import zeytin.common.message.editor_to_engine.enable_if_request;

namespace {
    std::string make_key(const std::string& variant_type, const std::string& key_path) {
        return variant_type + ":" + key_path;
    }

    std::string normalize_method_name(std::string value) {
        auto trim = [](std::string& text) {
            text.erase(text.begin(), std::find_if(text.begin(), text.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));
            text.erase(std::find_if(text.rbegin(), text.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), text.end());
        };

        trim(value);
        if (value.empty()) {
            return {};
        }

        const char first = value.front();
        const char last = value.back();
        if ((first == '\"' && last == '\"') || (first == '\'' && last == '\'')) {
            value = value.substr(1, value.size() - 2);
        }

        trim(value);
        if (value.ends_with("()")) {
            value.erase(value.size() - 2);
            trim(value);
        }

        return value;
    }
}

struct EnableIfTracker::Impl {
    struct Entry {
        bool enabled = false;
        bool has_value = false;
        std::string method_name;
        std::chrono::steady_clock::time_point last_request{};
    };

    std::unordered_map<uint64_t, std::unordered_map<std::string, Entry>> entries;
    std::chrono::milliseconds request_interval{100};
};

EnableIfTracker::EnableIfTracker()
    : pImpl(std::make_unique<Impl>()) {
}

EnableIfTracker::~EnableIfTracker() = default;

std::optional<bool> EnableIfTracker::get_value(uint64_t entity_id,
                                               const std::string& variant_type,
                                               const std::string& key_path,
                                               const std::string& method_name) {
    const std::string normalized = normalize_method_name(method_name);
    if (normalized.empty()) {
        return std::nullopt;
    }

    const std::string key = make_key(variant_type, key_path);
    auto& entity_entries = pImpl->entries[entity_id];
    auto& entry = entity_entries[key];

    const auto now = std::chrono::steady_clock::now();
    const bool method_changed = entry.method_name != normalized;
    const bool should_request = method_changed || !entry.has_value ||
        (entry.last_request.time_since_epoch().count() != 0 && (now - entry.last_request) >= pImpl->request_interval);

    if (should_request) {
        entry.method_name = normalized;
        entry.last_request = now;
        send_message_to_engine<EnableIfRequestMessage>(
            entity_id,
            variant_type,
            key_path,
            normalized
        );
    }

    if (!entry.has_value) {
        return std::nullopt;
    }

    return entry.enabled;
}

void EnableIfTracker::handle_result(uint64_t entity_id,
                                    const std::string& variant_type,
                                    const std::string& key_path,
                                    bool enabled) {
    const std::string key = make_key(variant_type, key_path);
    auto& entry = pImpl->entries[entity_id][key];
    entry.enabled = enabled;
    entry.has_value = true;
}
