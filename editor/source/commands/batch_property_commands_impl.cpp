module;

#include "rapidjson/document.h"
#include "rapidjson/pointer.h"

#include <cassert>
#include <variant>
#include <format>
#include <string>
#include <memory>
#include <algorithm>
#include <optional>
#include <vector>

module zeytin.command.batch_property;
import zeytin.selection;
import zeytin.engine.message;
import zeytin.common.message.editor_to_engine.entity_property_changed;
import zeytin.entity.document;
import zeytin.logger;
import zeytin.entity.registry;
import zeytin.entity.list;

struct BatchPropertyChangeCommand::Impl {
    uint64_t entity_id;
    std::string variant_type;
    std::vector<PropertyChange> changes;
    
    void apply_changes(bool use_new_values);
    bool update_entity_document(const PropertyChange& change);
    void notify_engine(const PropertyChange& change);
    
    std::string value_to_string(const PropertyValue& value) const;
    std::string get_key_type(const PropertyValue& value) const;
};

BatchPropertyChangeCommand::BatchPropertyChangeCommand(
    uint64_t entity_id,
    std::string variant_type,
    std::vector<PropertyChange> changes
) : pImpl(std::make_unique<Impl>())
{
    pImpl->entity_id = entity_id;
    pImpl->variant_type = std::move(variant_type);
    pImpl->changes = std::move(changes);
}

BatchPropertyChangeCommand::~BatchPropertyChangeCommand() = default;

void BatchPropertyChangeCommand::execute() {
    pImpl->apply_changes(true);
}

void BatchPropertyChangeCommand::undo() {
    pImpl->apply_changes(false);
}

void BatchPropertyChangeCommand::Impl::apply_changes(bool use_new_values) {
    for (const auto& change : changes) {
        PropertyChange modified_change = change;

		// no errors
        const bool valid = update_entity_document(use_new_values ? 
												  		 modified_change : 
												         PropertyChange{change.key_path, change.new_value, change.old_value});
        
        if (valid) {
            notify_engine(use_new_values ? 
						         modified_change : 
                                 PropertyChange{change.key_path, change.new_value, change.old_value});
        }
    }
}

bool BatchPropertyChangeCommand::Impl::update_entity_document(const PropertyChange& change) {
    auto entity_list_opt = EntityRegistry::get().get_entity_list();
    if (!entity_list_opt) {
        log_error("BatchPropertyChangeCommand: EntityList not registered in EntityRegistry");
        return false;
    }
    
    EntityList& entity_list = entity_list_opt->get();
    
    auto entity_opt = entity_list.find_entity_by_id(entity_id);
    if (!entity_opt) {
        log_error("BatchPropertyChangeCommand: Failed to find entity with ID {}", entity_id);
        return false;
    }
    
    EntityDocument& entity = entity_opt->get();
    rapidjson::Document& doc = entity.get_document();

    if(!entity.is_valid()) {
        log_error("Entity is invalid! Stopping here.");
        return false;
    }
    
    rapidjson::Value& variants = doc["variants"];
    for (auto& variant : variants.GetArray()) {
        if (!variant.HasMember("type")) {
            continue;
        }
        
        std::string type = variant["type"].GetString();
        if (type != variant_type) {
            continue;
        }
        
        // the path should be "/value/property_name" for components with value wrapper
        std::string pointer_path = "/value/" + change.key_path;
        std::replace(pointer_path.begin(), pointer_path.end(), '.', '/');
        
        rapidjson::Pointer pointer(pointer_path.c_str());
        rapidjson::Value* target = pointer.Get(variant);
        
        if (!target) {
            log_error("Failed to find property at path: {}", pointer_path);
            return false;
        }
        
        std::visit([target, &doc](auto&& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, int>) {
                target->SetInt(value);
            } else if constexpr (std::is_same_v<T, float>) {
                target->SetFloat(value);
            } else if constexpr (std::is_same_v<T, bool>) {
                target->SetBool(value);
            } else if constexpr (std::is_same_v<T, std::string>) {
                target->SetString(value.c_str(), doc.GetAllocator());
            }
        }, change.new_value);
        
        return true;
    }
    
    log_error("Variant '{}' not found on entity {}", variant_type, entity_id);
    return false;
}

// TODO: make this batch also ?
void BatchPropertyChangeCommand::Impl::notify_engine(const PropertyChange& change) {
    const std::string value = value_to_string(change.new_value);
    send_message_to_engine<EditorEntityPropertyChangedMessage>(
        entity_id,
        variant_type,
        get_key_type(change.new_value),
        change.key_path,
        value
    );
}

std::string BatchPropertyChangeCommand::Impl::value_to_string(const PropertyValue& value) const {
    return std::visit([](auto&& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return v;
        } else if constexpr (std::is_same_v<T, bool>) {
            return v ? "true" : "false";
        } else {
            return std::to_string(v);
        }
    }, value);
}

std::string BatchPropertyChangeCommand::Impl::get_key_type(const PropertyValue& value) const {
    return std::visit([](auto&& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, int>) {
            return "int";
        } else if constexpr (std::is_same_v<T, float>) {
            return "float";
        } else if constexpr (std::is_same_v<T, bool>) {
            return "bool";
        } else if constexpr (std::is_same_v<T, std::string>) {
            return "string";
        }
        return "unknown";
    }, value);
}

std::optional<std::string> BatchPropertyChangeCommand::get_description() const {
    if (pImpl->changes.empty()) {
        return std::nullopt;
    }
    
    if (pImpl->changes.size() == 1) {
        return std::format("Change {}.{}", pImpl->variant_type, pImpl->changes[0].key_path);
    }
    
    return std::format("Change {} properties of {}", pImpl->changes.size(), pImpl->variant_type);
}
