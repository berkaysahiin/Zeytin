module;

#include <functional>
#include <vector>
#include <memory>
#include <optional>

module zeytin.selection;
import zeytin.entity.registry;
import zeytin.logger;
import zeytin.level;
import zeytin.engine.message;
import zeytin.common.message.editor_to_engine.entity_selected;

struct SelectionManager::Impl {
	//TODO: change this to EntityID or not ?
    const EntityDocument* selected_entity = nullptr;
    SelectionType selection_type = SelectionType::None;
    std::vector<SelectionChangedCallback> callbacks;
    
    void notify_selection_changed() {
        for (auto& callback : callbacks) {
            callback();
        }
        send_selection_to_engine();
    }
    
    void send_selection_to_engine() {
        const uint64_t entity_id = selected_entity ? selected_entity->get_id() : 0;
        send_message_to_engine<EditorEntitySelectedMessage>(entity_id);
    }
};

SelectionManager::SelectionManager() 
    : pImpl(std::make_unique<Impl>()) {
	auto entity_list_opt = EntityRegistry::get().get_entity_list();
    if (entity_list_opt.has_value()) {
        entity_list_opt->get().add_level_unloading_callback([this](const Level&) {
            clear_selection();
        });
    }
}

SelectionManager::~SelectionManager() = default;

void SelectionManager::select_entity(const EntityDocument* entity) {
	if(entity == nullptr) {
		log_error("[SelectionManager::select_entity] Entity is nullptr");
		return;
	}

    if (pImpl->selected_entity == entity) {
		log_trace("[SelectionManager::select_entity] Already selected the same entity. Skipping");
        return;
    }
    
    pImpl->selected_entity = entity;
    pImpl->selection_type = SelectionType::Entity;
    pImpl->notify_selection_changed();
}

void SelectionManager::select_entity(const EntityID entity_id) {
	const EntityDocument *selected_entity = pImpl->selected_entity;
    if (selected_entity && selected_entity->get_id() == entity_id) {
        return; // already selected
    }

	auto entity_opt = EntityRegistry::get().find_entity(entity_id);

	if(!entity_opt) {
		log_error("Cannot find selected entity {}", entity_id);
		return;
	}
	
	const EntityDocument& entity = entity_opt->get();

    pImpl->selected_entity = &entity;
    pImpl->selection_type = SelectionType::Entity;
    pImpl->notify_selection_changed();
}

void SelectionManager::clear_selection() {
    pImpl->selected_entity = nullptr;
    pImpl->selection_type = SelectionType::None;
    pImpl->notify_selection_changed();
}

std::optional<EntityID> SelectionManager::get_selected_entity() const {
    const EntityDocument* entity_doc = pImpl->selected_entity;
	if(!entity_doc) {
		return std::nullopt;
	}
	return entity_doc->get_id();
}

EntityDocument* SelectionManager::get_selected_entity_unsafe() const {
	return const_cast<EntityDocument*>(pImpl->selected_entity);
}

bool SelectionManager::is_selected(const EntityID entity_id) const {
    if (!pImpl->selected_entity) {
        return false;
    }
    return pImpl->selected_entity->get_id() == entity_id;
}

SelectionType SelectionManager::get_selection_type() const {
    return pImpl->selection_type;
}

bool SelectionManager::has_selection() const {
    return pImpl->selection_type != SelectionType::None;
}

void SelectionManager::add_selection_changed_callback(SelectionChangedCallback callback) {
    pImpl->callbacks.push_back(std::move(callback));
}
