module;

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <string>
#include <memory>
#include <optional>
#include <cstdint>
#include <algorithm>
#include <vector>

module zeytin.command.component;
import zeytin.entity.registry;
import zeytin.entity.list;
import zeytin.entity.document;
import zeytin.component.document;
import zeytin.component.registry;
import zeytin.engine.message;
import zeytin.common.message.editor_to_engine.entity_variant_added;
import zeytin.common.message.editor_to_engine.entity_variant_removed;
import zeytin.logger;

namespace {
    void notify_engine_component_added(uint64_t entity_id, const std::string& type, const rapidjson::Value* component_data = nullptr) {
        std::string component_json;
        if (component_data) {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            component_data->Accept(writer);
            component_json.assign(buffer.GetString(), buffer.GetSize());
        }

        send_message_to_engine<EditorEntityVariantAddedMessage>(entity_id, type, component_json);
    }
    
    void notify_engine_component_removed(uint64_t entity_id, const std::string& type) {
        send_message_to_engine<EditorEntityVariantRemovedMessage>(entity_id, type);
    }

    EntityDocument* get_entity(uint64_t entity_id) {
        auto entity_list_opt = EntityRegistry::get().get_entity_list();
        if (!entity_list_opt) {
            log_error("ComponentCommand: EntityList not registered");
            return nullptr;
        }
        
        EntityList& entity_list = entity_list_opt->get();
        auto entity_opt = entity_list.find_entity_by_id(entity_id);
        
        if (!entity_opt) {
            log_error("ComponentCommand: Failed to find entity with ID {}", entity_id);
            return nullptr;
        }
        
        return &entity_opt->get();
    }

    ComponentDocument* get_variant_template(const std::string& component_type) {
        auto doc_opt = get_document_by_name(component_type);
        if (!doc_opt) {
            log_error("ComponentCommand: Failed to find component template: {}", component_type);
            return nullptr;
        }
        
        ComponentDocument& doc = doc_opt->get();
        if (doc.is_dead()) {
            log_error("ComponentCommand: Component template is dead: {}", component_type);
            return nullptr;
        }
        
        return &doc;
    }

    bool add_component_to_entity(uint64_t entity_id, const std::string& component_type) {
        EntityDocument* entity = get_entity(entity_id);
        if (!entity) return false;
        
        ComponentDocument* variant_template = get_variant_template(component_type);
        if (!variant_template) return false;
        
        rapidjson::Document& entity_doc = entity->get_document();
        const rapidjson::Document& variant_doc = variant_template->get_document();
        rapidjson::Value& variants_array = entity->get_components_json();
        
        // Check if component already exists
        for (const auto& v : variants_array.GetArray()) {
            if (v.HasMember("type") && std::string(v["type"].GetString()) == component_type) {
                log_warning("Component {} already exists on entity {}", component_type, entity_id);
                return false;
            }
        }
        
        // Copy variant template to entity
        rapidjson::Value new_component;
        new_component.CopyFrom(variant_doc, entity_doc.GetAllocator());
        if (new_component.HasMember("annotations")) {
            new_component.RemoveMember("annotations");
        }
        variants_array.PushBack(new_component, entity_doc.GetAllocator());
        
        // Notify engine
        notify_engine_component_added(entity_id, component_type);
        
        log_trace("Added component {} to entity {}", component_type, entity_id);
        return true;
    }

    bool remove_component_from_entity(uint64_t entity_id, const std::string& component_type) {
        EntityDocument* entity = get_entity(entity_id);
        if (!entity) return false;
        
        rapidjson::Value& variants_array = entity->get_components_json();
        
        // Find and remove the component
        for (rapidjson::SizeType i = 0; i < variants_array.Size(); ++i) {
            const auto& variant = variants_array[i];
            if (variant.HasMember("type") && 
                std::string(variant["type"].GetString()) == component_type) {
                
                variants_array.Erase(variants_array.Begin() + i);
                notify_engine_component_removed(entity_id, component_type);
                
                log_trace("Removed component {} from entity {}", component_type, entity_id);
                return true;
            }
        }
        
        log_warning("Component {} not found on entity {}", component_type, entity_id);
        return false;
    }
}

struct AddComponentCommand::Impl {
    uint64_t entity_id;
    std::string component_type;
    bool succeeded = false;
};

AddComponentCommand::AddComponentCommand(
    uint64_t entity_id, 
    const std::string& component_type
) : pImpl(std::make_unique<Impl>()) {
    pImpl->entity_id = entity_id;
    pImpl->component_type = component_type;
}

AddComponentCommand::~AddComponentCommand() = default;

void AddComponentCommand::execute() {
    pImpl->succeeded = add_component_to_entity(pImpl->entity_id, pImpl->component_type);
}

void AddComponentCommand::undo() {
    if (!pImpl->succeeded) {
        log_warning("Skipping undo for failed AddComponentCommand");
        return;
    }
    remove_component_from_entity(pImpl->entity_id, pImpl->component_type);
}

std::optional<std::string> AddComponentCommand::get_description() const {
    return "Add component: " + pImpl->component_type;
}

struct RemoveComponentCommand::Impl {
    uint64_t entity_id;
    std::string component_type;
    rapidjson::Document component_backup;
    bool succeeded = false;
};

RemoveComponentCommand::RemoveComponentCommand(
    uint64_t entity_id, 
    const std::string& component_type
) : pImpl(std::make_unique<Impl>()) {
    pImpl->entity_id = entity_id;
    pImpl->component_type = component_type;
}

RemoveComponentCommand::~RemoveComponentCommand() = default;

void RemoveComponentCommand::execute() {
    EntityDocument* entity = get_entity(pImpl->entity_id);
    if (!entity) {
        pImpl->succeeded = false;
        return;
    }
    
    rapidjson::Value& variants_array = entity->get_components_json();
    
    // Find and backup the component before removing
    for (rapidjson::SizeType i = 0; i < variants_array.Size(); ++i) {
        const auto& variant = variants_array[i];
        if (variant.HasMember("type") && 
            std::string(variant["type"].GetString()) == pImpl->component_type) {
            
            // Backup for undo
            pImpl->component_backup.CopyFrom(variant, pImpl->component_backup.GetAllocator());
            
            // Remove from entity
            variants_array.Erase(variants_array.Begin() + i);
            notify_engine_component_removed(pImpl->entity_id, pImpl->component_type);
            
            log_trace("Removed component {} from entity {}", pImpl->component_type, pImpl->entity_id);
            pImpl->succeeded = true;
            return;
        }
    }
    
    log_warning("Component {} not found on entity {}", pImpl->component_type, pImpl->entity_id);
    pImpl->succeeded = false;
}

void RemoveComponentCommand::undo() {
    if (!pImpl->succeeded) {
        log_warning("Skipping undo for failed RemoveComponentCommand");
        return;
    }
    
    EntityDocument* entity = get_entity(pImpl->entity_id);
    if (!entity) return; // get_entity already logs
    
    rapidjson::Document& entity_doc = entity->get_document();
    rapidjson::Value& variants_array = entity->get_components_json();
    
    // Restore from backup
    rapidjson::Value restored_variant;
    restored_variant.CopyFrom(pImpl->component_backup, entity_doc.GetAllocator());
    variants_array.PushBack(restored_variant, entity_doc.GetAllocator());
    
    notify_engine_component_added(pImpl->entity_id, pImpl->component_type, &pImpl->component_backup);
    
    log_trace("Restored component {} to entity {}", pImpl->component_type, pImpl->entity_id);
}

std::optional<std::string> RemoveComponentCommand::get_description() const {
    return "Remove component: " + pImpl->component_type;
}
