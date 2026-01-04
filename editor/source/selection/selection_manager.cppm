module;

#include <functional>
#include <memory>
#include <optional>

export module zeytin.selection;
import zeytin.singleton;
import zeytin.entity.document;

export enum class SelectionType {
    None,
    Entity
};

export class SelectionManager : public Singleton<SelectionManager> {
    friend class Singleton<SelectionManager>;
public:
    using SelectionChangedCallback = std::function<void()>;

	// Have to be defined in the translation unit even though its default since Impl is incomplete. Weird shit
    ~SelectionManager();
    
    void select_entity(const EntityDocument *entity);
    void select_entity(const EntityID entity_id);
    void clear_selection();
    
	std::optional<EntityID> get_selected_entity() const;
	EntityDocument* get_selected_entity_unsafe() const;
    
    bool is_selected(const EntityID entity_id) const;
    SelectionType get_selection_type() const;
    bool has_selection() const;
    
    void add_selection_changed_callback(SelectionChangedCallback callback);

private:
    SelectionManager();
    
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
