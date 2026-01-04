module;

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <memory>
#include <optional>
#include <string>

module zeytin.command.entity;
import zeytin.entity.document;
import zeytin.entity.list;
import zeytin.selection;
import zeytin.engine.event;
import zeytin.logger;
import zeytin.entity.registry;

namespace {
    void notify_entity_added(uint64_t entity_id, const std::string& entity_json) {
        rapidjson::Document msg;
        msg.SetObject();
        auto& alloc = msg.GetAllocator();

        msg.AddMember("type", "entity_added", alloc);
        msg.AddMember("entity_id", entity_id, alloc);
        
        rapidjson::Document entity_data;
        entity_data.Parse(entity_json.c_str());
        rapidjson::Value entity_copy;
        entity_copy.CopyFrom(entity_data, alloc);
        msg.AddMember("entity_data", entity_copy, alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        msg.Accept(writer);

        EngineEventBus::get().publish<const std::string&>(
            EngineEvent::EntityModifiedEditor, 
            buffer.GetString()
        );
    }

    void notify_entity_removed(uint64_t entity_id) {
        rapidjson::Document msg;
        msg.SetObject();
        auto& alloc = msg.GetAllocator();

        msg.AddMember("type", "entity_removed", alloc);
        msg.AddMember("entity_id", entity_id, alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        msg.Accept(writer);

        EngineEventBus::get().publish<const std::string&>(
            EngineEvent::EntityModifiedEditor, 
            buffer.GetString()
        );
    }
}

struct AddEntityCommand::Impl {
    std::string entity_name;
    uint64_t entity_id = 0;
    bool succeeded = false;
};

AddEntityCommand::~AddEntityCommand() = default;

AddEntityCommand::AddEntityCommand(std::string entity_name)
    : pImpl(std::make_unique<Impl>()) {
    pImpl->entity_name = std::move(entity_name);  
}

void AddEntityCommand::execute() {
    auto entity_list_opt = EntityRegistry::get().get_entity_list();
    if(!entity_list_opt.has_value()) {
        pImpl->succeeded = false;
        return;
    }

    if (pImpl->entity_id == 0) {
        pImpl->entity_id = EntityRegistry::get().generate_entity_id();
    }

    // check if entity already exists, from undo/redo
    auto entity_doc_opt = EntityRegistry::get().find_entity(pImpl->entity_id);
    if (entity_doc_opt.has_value()) {
        // entity exists (was undone) so just resurrect it
        EntityDocument& entity = entity_doc_opt->get();
        entity.mark_as_alive();
        
        std::string entity_json = entity.as_string();
        notify_entity_added(pImpl->entity_id, entity_json);
        
        log_trace("Resurrected entity '{}' with ID {}", pImpl->entity_name, pImpl->entity_id);
        pImpl->succeeded = true;
        
        SelectionManager::get().select_entity(&entity);
        return;
    }

    // create new entity (first time executing)
    rapidjson::Document new_doc;
    new_doc.SetObject();
    auto& allocator = new_doc.GetAllocator();
    
    new_doc.AddMember("entity_id", pImpl->entity_id, allocator);
    rapidjson::Value variants_array(rapidjson::kArrayType);
    new_doc.AddMember("variants", variants_array, allocator);

    EntityDocument entity(std::move(new_doc), pImpl->entity_name);
    std::string entity_json = entity.as_string();

    auto& entities = entity_list_opt->get().get_entities();
    entities.push_back(std::move(entity));

    // Notify engine
    notify_entity_added(pImpl->entity_id, entity_json);

    log_trace("Added entity '{}' with ID {}", pImpl->entity_name, pImpl->entity_id);
    pImpl->succeeded = true;
    
    SelectionManager::get().select_entity(&entities.back());
}


void AddEntityCommand::undo() {
    if (!pImpl->succeeded) {
        log_warning("Skipping undo for failed AddEntityCommand");
        return;
    }

	auto entity_doc_opt = EntityRegistry::get().find_entity(pImpl->entity_id);
	if(!entity_doc_opt.has_value()) {
    	return;
	}

	EntityDocument& entity = entity_doc_opt->get();

    // Check if entity was selected
    if (SelectionManager::get().is_selected(entity.get_id())) {
        SelectionManager::get().clear_selection();
    }

    // Mark as dead
    entity.mark_as_dead();

    // Notify engine
    notify_entity_removed(pImpl->entity_id);

    log_trace("Removed entity '{}' (ID: {})", pImpl->entity_name, pImpl->entity_id);
}

std::optional<std::string> AddEntityCommand::get_description() const {
    return "Add entity: " + pImpl->entity_name;
}

/// -------------------------------------------------------------------------------------------------------

struct RemoveEntityCommand::Impl {
    uint64_t entity_id;
    std::string entity_name;  // store for description
    bool succeeded = false;
};

RemoveEntityCommand::~RemoveEntityCommand() = default;

RemoveEntityCommand::RemoveEntityCommand(uint64_t entity_id)
    : pImpl(std::make_unique<Impl>()) {
    pImpl->entity_id = entity_id;
    
    // capture entity name for description
    auto entity_opt = EntityRegistry::get().find_entity(entity_id);
    if (entity_opt.has_value()) {
        pImpl->entity_name = entity_opt->get().get_name();
    }
}

void RemoveEntityCommand::execute() {
    auto entity_doc_opt = EntityRegistry::get().find_entity(pImpl->entity_id);
    if (!entity_doc_opt.has_value()) {
        log_warning("Cannot remove entity {}: not found", pImpl->entity_id);
        pImpl->succeeded = false;
        return;
    }

    EntityDocument& entity = entity_doc_opt->get();

    // check if entity was selected
    if (SelectionManager::get().is_selected(entity.get_id())) {
        SelectionManager::get().clear_selection();
    }

    entity.mark_as_dead();

    notify_entity_removed(pImpl->entity_id);

    log_trace("Removed entity '{}' (ID: {})", entity.get_name(), pImpl->entity_id);
    pImpl->succeeded = true;
}

void RemoveEntityCommand::undo() {
    if (!pImpl->succeeded) {
        log_warning("Skipping undo for failed RemoveEntityCommand");
        return;
    }

    auto entity_doc_opt = EntityRegistry::get().find_entity(pImpl->entity_id);
    if (!entity_doc_opt.has_value()) {
        log_error("Cannot restore entity {}: not found in list", pImpl->entity_id);
        return;
    }

    // entity exists (was removed), just resurrect it
    EntityDocument& entity = entity_doc_opt->get();
    entity.mark_as_alive();

	SelectionManager::get().select_entity(&entity);

    // Notify engine
    std::string entity_json = entity.as_string();
    notify_entity_added(pImpl->entity_id, entity_json);

    log_trace("Restored entity '{}' with ID {}", entity.get_name(), pImpl->entity_id);
}

std::optional<std::string> RemoveEntityCommand::get_description() const {
    return "Remove entity: " + pImpl->entity_name;
}
