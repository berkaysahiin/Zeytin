module;

#include <functional>
#include <vector>
#include <memory>
#include <optional>

module zeytin.selection;

struct SelectionManager::Impl {
	//TODO: change this to EntityID
    EntityDocument* selected_entity = nullptr;
    SelectionType selection_type = SelectionType::None;
    std::vector<SelectionChangedCallback> callbacks;
    
    void notify_selection_changed() {
        for (auto& callback : callbacks) {
            callback();
        }
    }
};

SelectionManager::SelectionManager() 
    : pImpl(std::make_unique<Impl>()) {
}

SelectionManager::~SelectionManager() = default;

void SelectionManager::select_entity(EntityDocument* entity) {
    if (pImpl->selected_entity == entity) {
        return;
    }
    
    pImpl->selected_entity = entity;
    pImpl->selection_type = entity ? SelectionType::Entity : SelectionType::None;
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
