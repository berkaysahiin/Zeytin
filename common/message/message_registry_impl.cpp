module;

#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "rapidjson/document.h"

module zeytin.common.message_registry;

MessageRegistry& MessageRegistry::get() {
    static MessageRegistry instance;
    return instance;
}

void MessageRegistry::register_handler(const std::string& type, MessageHandler handler) {
    if (type.empty()) {
        return;
    }

    std::unique_lock<std::shared_mutex> lock(m_mutex);
    m_handlers[type] = std::move(handler);
}

bool MessageRegistry::dispatch(const rapidjson::Document& doc, const std::string& raw) const {
    if (!doc.HasMember("type") || !doc["type"].IsString()) {
        return false;
    }

    const std::string type = doc["type"].GetString();
    if (type.empty()) {
        return false;
    }

    std::shared_lock<std::shared_mutex> lock(m_mutex);
    auto it = m_handlers.find(type);
    if (it == m_handlers.end()) {
        return false;
    }

    it->second(doc, raw);
    return true;
}
