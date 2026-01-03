module;

#include <cstdint>
#include <functional>
#include <vector>

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

    void select_entity(EntityDocument* entity) {
        if (m_selected_entity == entity) return;
        m_selected_entity = entity;
        m_selection_type = entity ? SelectionType::Entity : SelectionType::None;
        notify_selection_changed();
    }

    void clear_selection() {
        m_selected_entity = nullptr;
        m_selection_type = SelectionType::None;
        notify_selection_changed();
    }

    EntityDocument* get_selected_entity() const { return m_selected_entity; }
    SelectionType get_selection_type() const { return m_selection_type; }
    bool has_selection() const { return m_selection_type != SelectionType::None; }

    void add_selection_changed_callback(SelectionChangedCallback callback) {
        m_callbacks.push_back(std::move(callback));
    }

private:
    SelectionManager() = default;

    void notify_selection_changed() {
        for (auto& callback : m_callbacks) {
            callback();
        }
    }

    EntityDocument* m_selected_entity = nullptr;
    SelectionType m_selection_type = SelectionType::None;
    std::vector<SelectionChangedCallback> m_callbacks;
};
