module;

#include "imgui.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

module zeytin.entity.view;

import zeytin.component;
import zeytin.component.instance;
import zeytin.component.registry;
import zeytin.entity;
import zeytin.entity.document;
import zeytin.entity.document.conversion;
import zeytin.entity.registry.runtime;
import zeytin.property;
import zeytin.selection;
import zeytin.utility.typedefs;

namespace {
    struct EntityViewState {
        Maybe<EntityID> selected_entity_id;
        Maybe<EntityID> cached_entity_id;
        Maybe<Entity> cached_entity;
    };

    List<EntityDocumentID> get_entity_document_ids_sorted();
    void render_entity_header(EntityID entity_id);
    void render_component_section(ConstRef<ComponentInstance> instance, ConstRef<Component> component, size_t index);
    void render_component_properties(ConstRef<ComponentInstance> instance);
    void render_component_properties_table(const List<Property>& properties, const String& table_id);

    void sync_selection(Ref<EntityViewState> state);
    void render_entity_list(Ref<EntityViewState> state);
    void render_entity_details(Ref<EntityViewState> state);
    void ensure_cached_entity(Ref<EntityViewState> state, EntityID entity_id);
    void clear_cached_entity(Ref<EntityViewState> state);
}

struct EntityViewWindow::Impl {
    EntityViewState state;
};

EntityViewWindow::EntityViewWindow()
    : pImpl(std::make_unique<Impl>()) {
}

EntityViewWindow::~EntityViewWindow() = default;

void EntityViewWindow::render() {
    sync_selection(pImpl->state);

    ImGui::BeginChild("EntityListPane", ImVec2(280, 0), true);
    render_entity_list(pImpl->state);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("EntityDetailsPane", ImVec2(0, 0), true);
    render_entity_details(pImpl->state);
    ImGui::EndChild();
}

namespace {

List<EntityDocumentID> get_entity_document_ids_sorted() {
    auto doc_ids = runtime_get_entity_document_ids();
    
    std::sort(doc_ids.begin(), doc_ids.end(), [](EntityDocumentID left_id, EntityDocumentID right_id) {
        auto left_doc = runtime_get_entity_document_const(left_id);
        auto right_doc = runtime_get_entity_document_const(right_id);
        
        if (!left_doc || !right_doc) {
            return left_doc.has_value();
        }
        
        return left_doc->get().get_name() < right_doc->get().get_name();
    });

    return doc_ids;
}

void render_entity_header(EntityID entity_id) {
    auto entity_opt = runtime_get_entity_const(entity_id);
    if (!entity_opt) {
        ImGui::Text("Unknown Entity");
        return;
    }
    
    ConstRef<Entity> entity = entity_opt->get();
    ImGui::Text("%s", entity.name.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("(ID: %llu)", static_cast<unsigned long long>(entity.id));
}

void render_component_section(ConstRef<ComponentInstance> instance, ConstRef<Component> component, const size_t index) {
    String header = component.type + " (" + std::to_string(index) + ")";

    if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextDisabled("Component ID: %lu", component.id);
        ImGui::TextDisabled("Instance ID: %lu", instance.id);
        ImGui::Separator();
        render_component_properties(instance);
    }
}

void render_component_properties(ConstRef<ComponentInstance> instance) {
    const List<Property> properties = instance_get_properties(instance);
    if (properties.empty()) {
        ImGui::TextDisabled("No properties on component");
        return;
    }

    const String table_id = "component_properties_" + std::to_string(instance.id);
    render_component_properties_table(properties, table_id);
}

void render_component_properties_table(const List<Property>& properties, const String& table_id) {
    if (!ImGui::BeginTable(table_id.c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        return;
    }

    ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 180.0f);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    for (const auto& prop : properties) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", prop.name.c_str());
        ImGui::TableNextColumn();
        const String value_str = property_value_to_string(prop.value);
        ImGui::TextWrapped("%s", value_str.c_str());
    }

    ImGui::EndTable();
}

void sync_selection(Ref<EntityViewState> state) {
    if (SelectionManager::get().has_selection()) {
        auto selected = SelectionManager::get().get_selected_entity();
        if (selected.has_value()) {
            if (!state.selected_entity_id.has_value() || state.selected_entity_id.value() != selected.value()) {
                state.selected_entity_id = selected;
                clear_cached_entity(state);
            }
            return;
        }
    }

    if (!state.selected_entity_id.has_value()) {
        return;
    }

    auto entity_opt = runtime_get_entity_const(state.selected_entity_id.value());
    if (!entity_opt) {
        state.selected_entity_id.reset();
        clear_cached_entity(state);
    }
}

void render_entity_list(Ref<EntityViewState> state) {
    ImGui::Text("Entities");
    ImGui::Separator();

    auto doc_ids = get_entity_document_ids_sorted();
    if (doc_ids.empty()) {
        ImGui::TextDisabled("No entities available");
        return;
    }

    for (EntityDocumentID doc_id : doc_ids) {
        auto entity_id_opt = runtime_get_entity_id_for_document(doc_id);
        if (!entity_id_opt) {
            continue;
        }
        
        EntityID entity_id = entity_id_opt.value();
        auto entity_opt = runtime_get_entity_const(entity_id);
        if (!entity_opt) {
            continue;
        }

        ConstRef<Entity> entity = entity_opt->get();
        const bool is_selected = state.selected_entity_id.has_value() && state.selected_entity_id.value() == entity_id;
        String label = entity.name;
        if (label.empty()) {
            label = "(unnamed)";
        }

        if (ImGui::Selectable(label.c_str(), is_selected)) {
            state.selected_entity_id = entity_id;
            clear_cached_entity(state);
            // Note: SelectionManager expects EntityDocument*, but we're moving away from that
            // For now, we just select by ID
            auto doc_opt = runtime_get_entity_document_const(doc_id);
            if (doc_opt) {
                SelectionManager::get().select_entity(&doc_opt->get());
            }
        }
    }
}

void render_entity_details(Ref<EntityViewState> state) {
    if (!state.selected_entity_id.has_value()) {
        ImGui::TextDisabled("Select an entity to view details");
        return;
    }

    EntityID entity_id = state.selected_entity_id.value();
    auto entity_opt = runtime_get_entity_const(entity_id);
    if (!entity_opt) {
        ImGui::TextDisabled("Selected entity missing");
        return;
    }

    if (state.cached_entity_id.has_value() && state.cached_entity_id.value() != entity_id) {
        clear_cached_entity(state);
    }
    
    render_entity_header(entity_id);
    ImGui::Separator();

    ensure_cached_entity(state, entity_id);
    if (!state.cached_entity.has_value()) {
        ImGui::TextDisabled("Entity conversion failed");
        return;
    }

    const Entity& entity = state.cached_entity.value();
    if (entity.components.empty()) {
        ImGui::TextDisabled("No components attached");
        return;
    }

    for (size_t index = 0; index < entity.components.size(); ++index) {
        auto instance_opt = get_instance_const(entity.components[index]);
        if (!instance_opt) {
            continue;
        }

        ConstRef<ComponentInstance> instance = instance_opt->get();
        auto component_opt = get_component_const(instance.component_id);
        if (!component_opt) {
            continue;
        }

        render_component_section(instance, component_opt->get(), index);
    }
}

void ensure_cached_entity(Ref<EntityViewState> state, EntityID entity_id) {
    if (state.cached_entity_id.has_value() && state.cached_entity_id.value() == entity_id && state.cached_entity.has_value()) {
        return;
    }

    auto entity_opt = runtime_get_entity_const(entity_id);
    if (!entity_opt) {
        return;
    }
    
    state.cached_entity = entity_opt->get();
    state.cached_entity_id = entity_id;
}

void clear_cached_entity(Ref<EntityViewState> state) {
    state.cached_entity.reset();
    state.cached_entity_id.reset();
}

}
