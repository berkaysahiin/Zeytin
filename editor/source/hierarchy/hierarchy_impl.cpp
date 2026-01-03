module;

#include "imgui.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <random>
#include <algorithm>
#include <fstream>
#include <filesystem>

module zeytin.hierarchy;
import zeytin.resource;
import zeytin.entity.list;
import zeytin.engine.event;
import zeytin.selection;

namespace {
    void notify_engine_entity_variant_added(uint64_t entity_id, const std::string& type) {
        rapidjson::Document msg;
        msg.SetObject();
        auto& alloc = msg.GetAllocator();

        msg.AddMember("type", "entity_variant_added", alloc);
        msg.AddMember("entity_id", entity_id, alloc);
        msg.AddMember("variant_type", rapidjson::Value(type.c_str(), alloc), alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        msg.Accept(writer);

        EngineEventBus::get().publish<const std::string&>(EngineEvent::EntityModifiedEditor, buffer.GetString());
    }

    void notify_engine_entity_variant_removed(uint64_t entity_id, const std::string& type) {
        rapidjson::Document msg;
        msg.SetObject();
        auto& alloc = msg.GetAllocator();

        msg.AddMember("type", "entity_variant_removed", alloc);
        msg.AddMember("entity_id", entity_id, alloc);
        msg.AddMember("variant_type", rapidjson::Value(type.c_str(), alloc), alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        msg.Accept(writer);

        EngineEventBus::get().publish<const std::string&>(EngineEvent::EntityModifiedEditor, buffer.GetString());
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

    if (ImGui::Button("+ Create New Entity", ImVec2(150, 20))) {
        memset(new_entity_name, 0, sizeof(new_entity_name));
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

        ImGui::Spacing();

        bool is_valid_name = strlen(new_entity_name) > 0;

        if (!is_valid_name) {
            ImGui::BeginDisabled();
        }

        if ((ImGui::Button("Create", ImVec2(120, 0)) || enter_pressed) && is_valid_name) {
            create_new_entity(new_entity_name);
            show_new_entity_popup = false;
            ImGui::CloseCurrentPopup();
        }

        if (!is_valid_name) {
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
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
    rapidjson::Document& document = entity_document.get_document();
    if (!document.IsObject()) {
        return;
    }

    uint64_t entity_id = 0;
    if (document.HasMember("entity_id") && document["entity_id"].IsUint64()) {
        entity_id = document["entity_id"].GetUint64();
    }

    const std::string& name = entity_document.get_name();
    if (name.empty()) {
        return;
    }

    ImGui::PushID(static_cast<int>(entity_id));

    bool is_selected = (SelectionManager::get().get_selected_entity() == &entity_document);

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
            render_add_variant_menu(entity_document);
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Duplicate")) {
            // TODO: implement duplication
        }

        if (ImGui::MenuItem("Delete")) {
            entity_document.mark_as_dead();
            notify_entity_removed(entity_id);
            
            if (SelectionManager::get().get_selected_entity() == &entity_document) {
                SelectionManager::get().clear_selection();
            }
        }

        ImGui::EndPopup();
    }
}

void Hierarchy::render_add_variant_menu(EntityDocument& entity_document) {
    for (const auto& variant : m_variants) {
        if (variant.is_dead() || variant.get_name().empty()) continue;

        const std::string& variant_name = variant.get_name();
        bool already_exists = check_variant_exists(entity_document, variant_name);

        if (already_exists) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::MenuItem(variant_name.c_str(), nullptr, false, false);
            ImGui::PopStyleColor();
        } else if (ImGui::MenuItem(variant_name.c_str())) {
            add_variant_to_entity(entity_document, const_cast<VariantDocument&>(variant));
        }
    }
}

bool Hierarchy::check_variant_exists(const EntityDocument& entity_document, const std::string& variant_name) {
    const rapidjson::Document& doc = entity_document.get_document();
    if (doc.HasMember("variants") && doc["variants"].IsArray()) {
        for (const auto& variant : doc["variants"].GetArray()) {
            if (std::string(variant["type"].GetString()) == variant_name) {
                return true;
            }
        }
    }
    return false;
}

void Hierarchy::add_variant_to_entity(EntityDocument& entity_document, VariantDocument& variant_document) {
    rapidjson::Document& entity_doc = entity_document.get_document();
    const rapidjson::Document& variant_doc = variant_document.get_document();
    
    uint64_t entity_id = entity_doc["entity_id"].GetUint64();

    if (!entity_doc.HasMember("variants")) {
        rapidjson::Value variants_array(rapidjson::kArrayType);
        entity_doc.AddMember("variants", variants_array, entity_doc.GetAllocator());
    }

    rapidjson::Value& entity_variants = entity_doc["variants"];
    const std::string& type = variant_doc["type"].GetString();

    for (const auto& v : entity_variants.GetArray()) {
        if (std::string(v["type"].GetString()) == type) {
            return;
        }
    }

    rapidjson::Value copied_variant(variant_doc, entity_doc.GetAllocator());
    entity_variants.PushBack(copied_variant, entity_doc.GetAllocator());
    notify_engine_entity_variant_added(entity_id, type);
    add_required_variants_to_entity(entity_document, type);
}

void Hierarchy::add_required_variants_to_entity(EntityDocument& entity_document, const std::string& variant_type) {
    std::filesystem::path requires_path = ResourceManager::get().get_components_paths() / "requires" / (variant_type + ".requires");

    if (!std::filesystem::exists(requires_path)) {
        return;
    }

    try {
        std::ifstream requires_file(requires_path);
        if (!requires_file.is_open()) {
            return;
        }

        std::string json_str((std::istreambuf_iterator<char>(requires_file)),
                              std::istreambuf_iterator<char>());
        requires_file.close();

        rapidjson::Document requires_doc;
        requires_doc.Parse(json_str.c_str());

        if (requires_doc.HasParseError() || !requires_doc.IsArray()) {
            return;
        }

        for (const auto& req : requires_doc.GetArray()) {
            if (!req.IsString()) continue;
            
            std::string required_type = req.GetString();
            
            if (check_variant_exists(entity_document, required_type)) continue;

            for (auto& variant : m_variants) {
                if (variant.get_name() == required_type) {
                    add_variant_to_entity(entity_document, variant);
                    break;
                }
            }
        }
    } catch (...) {
        // Silent fail
    }
}
