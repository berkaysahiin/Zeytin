module;

#include "imgui.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include <random>
#include <algorithm>

module zeytin.hierarchy;
import zeytin.entity.list;
import zeytin.engine.event;
import zeytin.selection;
import zeytin.command.manager;
import zeytin.command.component;
import zeytin.command.entity;

import zeytin.validation;
import zeytin.validation.entity;

namespace {
    void notify_entity_removed(uint64_t entity_id) {
        rapidjson::Document msg;
        msg.SetObject();
        auto& alloc = msg.GetAllocator();

        msg.AddMember("type", "entity_removed", alloc);
        msg.AddMember("entity_id", entity_id, alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        msg.Accept(writer);

        EngineEventBus::get().publish<const std::string&>(EngineEvent::EntityModifiedEditor, buffer.GetString());
    }
}

Hierarchy::Hierarchy(std::vector<VariantDocument>& variants, EntityList* entity_list)
    : m_variants(variants), m_entity_list(entity_list)
{
    subscribe_events();
}

void Hierarchy::subscribe_events() {

}

void Hierarchy::update() {
    render_create_entity();
    render_save_controls();
    ImGui::Separator();

    if (!m_entity_list) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Entity list not initialized");
        return;
    }

    auto& entities = m_entity_list->get_entities();
    for (auto& entity : entities) {
        if (!entity.is_dead()) {
            render_entity(entity);
        }
    }
}

void Hierarchy::render_save_controls() {
    static bool save_real_time = false;
    static float save_interval = 1.0f;
    static float time_since_last_save = 0.0f;

    if (save_real_time) {
        time_since_last_save += ImGui::GetIO().DeltaTime;
    }

    if (ImGui::Button("Save All")) {
        save_all_entities();
        time_since_last_save = 0.0f;
    }

    ImGui::SameLine();
    if (ImGui::Checkbox("Save Realtime", &save_real_time)) {
        time_since_last_save = 0.0f;
    }

    if (save_real_time && time_since_last_save >= save_interval) {
        save_all_entities();
        time_since_last_save = 0.0f;
    }
}

void Hierarchy::save_all_entities() {
    if (m_entity_list) {
        m_entity_list->save_all_entities();
    }
}

void Hierarchy::render_create_entity() {
    constexpr size_t MAX_ENTITY_NAME_LENGTH = 128;
    static char new_entity_name[MAX_ENTITY_NAME_LENGTH] = "";
    static bool show_new_entity_popup = false;
    static std::string validation_error = "";

    if (ImGui::Button("+ Create New Entity", ImVec2(150, 20))) {
        memset(new_entity_name, 0, sizeof(new_entity_name));
        validation_error = "";
        show_new_entity_popup = true;
    }

    ImGui::Spacing();

    if (show_new_entity_popup) {
        ImGui::OpenPopup("New Entity");
    }

    if (ImGui::BeginPopupModal("New Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter a name for the new entity:");

        bool enter_pressed = ImGui::InputTextWithHint(
            "##EntityName",
            "Entity name",
            new_entity_name,
            sizeof(new_entity_name),
            ImGuiInputTextFlags_EnterReturnsTrue
        );

        if (!validation_error.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", validation_error.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();

        bool create_clicked = ImGui::Button("Create", ImVec2(120, 0)) || enter_pressed;

        if (create_clicked) {
            std::string name_str(new_entity_name);
            ValidationResult validation = entity_validation::validate_entity_name(name_str, *m_entity_list);
            
            if (validation.is_valid()) {
                auto command = std::make_unique<AddEntityCommand>(name_str);
                CommandManager::get().execute_command(std::move(command));
                validation_error = "";
                show_new_entity_popup = false;
                ImGui::CloseCurrentPopup();
            } else {
                validation_error = validation.get_all_errors();
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            validation_error = "";
            show_new_entity_popup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Hierarchy::create_new_entity(const char* name) {
    std::string safe_name = name;
    safe_name.erase(std::remove_if(safe_name.begin(), safe_name.end(),
        [](char c) { return c == '/' || c == '\\' || c == ':' || c == '*' || 
                            c == '?' || c == '"' || c == '<' || c == '>' || c == '|'; }), 
        safe_name.end());

    auto& entities = m_entity_list->get_entities();
    for (auto& entity : entities) {
        if (!entity.is_dead() && entity.get_name() == safe_name) {
            return;
        }
    }

    SelectionManager::get().clear_selection();

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    uint64_t uuid = dis(gen);
    
    rapidjson::Document new_doc;
    new_doc.SetObject();
    auto& allocator = new_doc.GetAllocator();
    
    new_doc.AddMember("entity_id", uuid, allocator);
    rapidjson::Value variantsArray(rapidjson::kArrayType);
    new_doc.AddMember("variants", variantsArray, allocator);

    EntityDocument entity(std::move(new_doc), safe_name);
    entities.push_back(std::move(entity));
}

void Hierarchy::render_entity(EntityDocument& entity_document) {
    const std::string& name = entity_document.get_name();
    if (name.empty()) {
        return;
    }

    uint64_t entity_id = entity_document.get_id();

    ImGui::PushID(static_cast<int>(entity_id));

    const bool is_selected = SelectionManager::get().is_selected(entity_document.get_id());

    float item_width = ImGui::GetContentRegionAvail().x;
    float item_height = ImGui::GetFrameHeight() + 4.0f;

    ImVec4 bg_color = is_selected 
        ? ImVec4(0.24f, 0.42f, 0.62f, 1.0f)
        : ImVec4(0.18f, 0.18f, 0.2f, 1.0f);

    ImVec4 bg_hover_color = is_selected
        ? ImVec4(0.28f, 0.46f, 0.66f, 1.0f)
        : ImVec4(0.25f, 0.25f, 0.28f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Header, bg_color);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, bg_hover_color);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.26f, 0.44f, 0.64f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 2.0f));

    std::string display_name = std::string("O ") + name;  

    bool clicked = ImGui::Selectable(display_name.c_str(), is_selected, 
                                      ImGuiSelectableFlags_None,
                                      ImVec2(item_width, item_height));

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    if (clicked) {
        SelectionManager::get().select_entity(&entity_document);
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        SelectionManager::get().select_entity(&entity_document);
        ImGui::OpenPopup("entity_context_menu");
    }

    handle_entity_context_menu(entity_document, entity_id);

    ImGui::PopID();
}

void Hierarchy::handle_entity_context_menu(EntityDocument& entity_document, uint64_t entity_id) {
    if (ImGui::BeginPopup("entity_context_menu")) {
        if (ImGui::BeginMenu("Add Component")) {
            render_add_component_menu(entity_document);
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Duplicate")) {
            // TODO: implement duplication
        }

        if (ImGui::MenuItem("Delete")) {
            entity_document.mark_as_dead();
            notify_entity_removed(entity_id);
            
            if (SelectionManager::get().is_selected(entity_document.get_id())) {
                SelectionManager::get().clear_selection();
            }
        }

        ImGui::EndPopup();
    }
}

void Hierarchy::render_add_component_menu(EntityDocument& entity_document) {
    uint64_t entity_id = entity_document.get_id();
    
    for (const auto& variant : m_variants) {
        if (variant.is_dead() || variant.get_name().empty()) continue;

        const std::string& component_name = variant.get_name();
        bool already_exists = entity_document.has_component(component_name);

        if (already_exists) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::MenuItem(component_name.c_str(), nullptr, false, false);
            ImGui::PopStyleColor();
        } else if (ImGui::MenuItem(component_name.c_str())) {
            auto command = std::make_unique<AddComponentCommand>(entity_id, component_name);
            CommandManager::get().execute_command(std::move(command));
        }
    }
}
