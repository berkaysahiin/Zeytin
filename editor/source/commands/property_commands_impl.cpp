module;

#include "rapidjson/pointer.h"

#include <cassert>
#include <variant>
#include <format>
#include <string>
#include <memory>
#include <algorithm>
#include <optional>

module zeytin.command.property;
import zeytin.selection;
import zeytin.engine.message;
import zeytin.common.message.editor_to_engine.entity_property_changed;
import zeytin.entity.document;
import zeytin.logger;
import zeytin.entity.registry;
import zeytin.entity.list;

struct PropertyChangeCommand::Impl {
    PropertyLocation location;
    PropertyValue old_value;
    PropertyValue new_value;
    
    void apply_value(const PropertyValue& value, const PropertyLocation& loc);
    bool update_entity_document(const PropertyValue& value, const PropertyLocation& loc);
    void notify_engine(const PropertyValue& value, const PropertyLocation& loc);
    
    std::string value_to_string(const PropertyValue& value) const;
    std::string get_key_type(const PropertyValue& value) const;
};

PropertyChangeCommand::PropertyChangeCommand(
    PropertyLocation location,
    PropertyValue old_value,
    PropertyValue new_value
) : pImpl(std::make_unique<Impl>())
{
    pImpl->location = std::move(location);
    pImpl->old_value = std::move(old_value);
    pImpl->new_value = std::move(new_value);
}

PropertyChangeCommand::~PropertyChangeCommand() = default;

void PropertyChangeCommand::execute() {
    pImpl->apply_value(pImpl->new_value, pImpl->location);
}

void PropertyChangeCommand::undo() {
    pImpl->apply_value(pImpl->old_value, pImpl->location);
}

void PropertyChangeCommand::Impl::apply_value(const PropertyValue& value, const PropertyLocation& loc) {
    if(update_entity_document(value, loc)) {
    	notify_engine(value, loc);
	}
}

bool PropertyChangeCommand::Impl::update_entity_document(const PropertyValue& value, const PropertyLocation& loc) {
    auto entity_list_opt = EntityRegistry::get().get_entity_list();
    if (!entity_list_opt) {
        log_error("PropertyChangeCommand: EntityList not registered in EntityRegistry");
        return false;
    }
    
    EntityList& entity_list = entity_list_opt->get();
    
    auto entity_opt = entity_list.find_entity_by_id(loc.entity_id);
    if (!entity_opt) {
        log_error("PropertyChangeCommand: Failed to find entity with ID {}", loc.entity_id);
        return false;
    }
    
    EntityDocument& entity = entity_opt->get();
    rapidjson::Document& doc = entity.get_document();

	if(!entity.is_valid()) {
		log_error("Entity is invalid!. Stopping here.");
		return false;
	}
    
    rapidjson::Value& variants = doc["variants"];
    for (auto& variant : variants.GetArray()) {
        if (!variant.HasMember("type")) {
            continue;
        }
        
        std::string type = variant["type"].GetString();
        if (type != loc.variant_type) {
            continue;
        }
        
        // The path should be "/value/property_name" for components with value wrapper
        std::string pointer_path = "/value/" + loc.key_path;
        std::replace(pointer_path.begin(), pointer_path.end(), '.', '/');
        
        rapidjson::Pointer pointer(pointer_path.c_str());
        rapidjson::Value* target = pointer.Get(variant);
        
        if (!target) {
            log_error("PropertyChangeCommand: Property '{}' not found at path on entity {}", 
                      loc.key_path, loc.entity_id);
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
    
    // If we reach here, the variant type was not found
    log_error("PropertyChangeCommand: Variant '{}' not found on entity {}", 
              loc.variant_type, loc.entity_id);
	return false;
}

void PropertyChangeCommand::Impl::notify_engine(const PropertyValue& value, const PropertyLocation& loc) {
    const std::string value_str = value_to_string(value);
    send_message_to_engine<EditorEntityPropertyChangedMessage>(
        loc.entity_id,
        loc.variant_type,
        get_key_type(value),
        loc.key_path,
        value_str
    );

    log_trace("Command send to engine");
}

std::string PropertyChangeCommand::Impl::value_to_string(const PropertyValue& value) const {
    // Convert variant value to string representation for engine communication
    return std::visit([](auto&& val) -> std::string {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, int>) {
            return std::to_string(val);
        } else if constexpr (std::is_same_v<T, float>) {
            return std::to_string(val);
        } else if constexpr (std::is_same_v<T, bool>) {
            return val ? "true" : "false";
        } else if constexpr (std::is_same_v<T, std::string>) {
            return val;
        }
    }, value);
}

std::string PropertyChangeCommand::Impl::get_key_type(const PropertyValue& value) const {
    // Determine the type name from the variant for engine communication
    return std::visit([](auto&& val) -> std::string {
        using T = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<T, int>) {
            return "int";
        } else if constexpr (std::is_same_v<T, float>) {
            return "float";
        } else if constexpr (std::is_same_v<T, bool>) {
            return "bool";
        } else if constexpr (std::is_same_v<T, std::string>) {
            return "string";
        }
    }, value);
}

std::optional<std::string> PropertyChangeCommand::get_description() const {
    return std::format("Change property \"{}\" on \"{}\"", pImpl->location.key_path, pImpl->location.variant_type);
}
