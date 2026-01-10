module;

#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "rapidjson/document.h"

export module zeytin.common.message_registry;

export using MessageHandler = std::function<void(const rapidjson::Document&, const std::string&)>;

export class MessageRegistry {
public:
    static MessageRegistry& get();

    void register_handler(const std::string& type, MessageHandler handler);
    bool dispatch(const rapidjson::Document& doc, const std::string& raw) const;

private:
    MessageRegistry() = default;

    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, MessageHandler> m_handlers;
};
