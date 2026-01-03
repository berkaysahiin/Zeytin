module;

#include "imgui.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <string>
#include <vector>
#include <map>
#include <algorithm>

module zeytin.inspector;
import zeytin.selection;
import zeytin.engine.event;
import zeytin.resource;

namespace {
    void notify_engine_entity_property_changed(uint64_t entity_id,
                                               const std::string& variant_type,
                                               const std::string& key_type,
                                               const std::string& key_path,
                                               const std::string& new_value) {
        rapidjson::Document msg;
        msg.SetObject();
        auto& alloc = msg.GetAllocator();

        msg.AddMember("type", "entity_property_changed", alloc);
        msg.AddMember("entity_id", entity_id, alloc);
        msg.AddMember("variant_type", rapidjson::Value(variant_type.c_str(), alloc), alloc);
        msg.AddMember("key_type", rapidjson::Value(key_type.c_str(), alloc), alloc);
        msg.AddMember("key_path", rapidjson::Value(key_path.c_str(), alloc), alloc);
        msg.AddMember("new_value", rapidjson::Value(new_value.c_str(), alloc), alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        msg.Accept(writer);

        EngineEventBus::get().publish<const std::string&>(EngineEvent::EntityModifiedEditor, buffer.GetString());
    }

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
}

Inspector::Inspector(std::vector<VariantDocument>& variants)
    : m_variants(variants)
{
}

void Inspector::render() {
    EntityDocument* selected = SelectionManager::get().get_selected_entity();

    if (!selected || selected->is_dead()) {
        ImGui::TextDisabled("No entity selected");
        return;
    }

    render_entity_header(*selected);
    ImGui::Separator();
    render_variants(*selected);
}

void Inspector::render_entity_header(EntityDocument& entity) {
    rapidjson::Document& doc = entity.get_document();
    uint64_t entity_id = 0;
    if (doc.HasMember("entity_id") && doc["entity_id"].IsUint64()) {
        entity_id = doc["entity_id"].GetUint64();
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Text("%s", entity.get_name().c_str());
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::TextDisabled("(ID: %llu)", static_cast<unsigned long long>(entity_id));
}

void Inspector::render_variants(EntityDocument& entity) {
    rapidjson::Document& doc = entity.get_document();

    uint64_t entity_id = 0;
    if (doc.HasMember("entity_id") && doc["entity_id"].IsUint64()) {
        entity_id = doc["entity_id"].GetUint64();
    }

    if (!doc.HasMember("variants") || !doc["variants"].IsArray()) {
        render_add_component_button(entity, entity_id);
        return;
    }

    rapidjson::Value& variants = doc["variants"];

    for (rapidjson::SizeType i = 0; i < variants.Size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        render_variant(doc, variants[i], static_cast<int>(i), entity_id);
        ImGui::PopID();
    }

    ImGui::Spacing();
    render_add_component_button(entity, entity_id);
}

void Inspector::render_variant(rapidjson::Document& document, rapidjson::Value& variant,
                                int index, uint64_t entity_id) {
    if (!variant.IsObject() || !variant.HasMember("type")) {
        return;
    }

    std::string variant_type = variant["type"].GetString();

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25f, 0.25f, 0.3f, 1.0f));

    bool is_open = ImGui::CollapsingHeader(variant_type.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    ImGui::PopStyleColor(3);

    if (ImGui::BeginPopupContextItem("variant_context")) {
        if (ImGui::MenuItem("Remove Component")) {
            notify_engine_entity_variant_removed(entity_id, variant_type);
            rapidjson::Value& variants_array = document["variants"];
            variants_array.Erase(variants_array.Begin() + index);
        }
        ImGui::EndPopup();
    }

    if (is_open) {
        ImGui::Indent(10.0f);

        for (auto it = variant.MemberBegin(); it != variant.MemberEnd(); ++it) {
            const char* key = it->name.GetString();
            
            if (strcmp(key, "type") == 0) continue;
            if (strcmp(key, "entity_id") == 0) continue;

            rapidjson::Value& value = it->value;
            std::string path = key;

            ImGui::PushID(key);

            // If it's "value" object, render its contents directly without tree node
            if (strcmp(key, "value") == 0 && value.IsObject()) {
                render_object(document, value, entity_id, variant_type, path);
            } else if (value.IsInt()) {
                render_int_field(document, value, entity_id, variant_type, key, path);
            } else if (value.IsFloat() || value.IsDouble()) {
                render_float_field(document, value, entity_id, variant_type, key, path);
            } else if (value.IsBool()) {
                render_bool_field(value, entity_id, variant_type, key, path);
            } else if (value.IsString()) {
                render_string_field(document, value, entity_id, variant_type, key, path);
            } else if (value.IsObject()) {
                if (ImGui::TreeNode(key)) {
                    render_object(document, value, entity_id, variant_type, path);
                    ImGui::TreePop();
                }
            } else if (value.IsArray()) {
                render_array_field(document, value, entity_id, variant_type, key, path);
            }

            ImGui::PopID();
        }

        ImGui::Unindent(10.0f);
    }

    ImGui::Spacing();
}

void Inspector::render_object(rapidjson::Document& document, rapidjson::Value& object,
                               uint64_t entity_id, const std::string& variant_type,
                               const std::string& parent_path) {
    for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it) {
        const char* key = it->name.GetString();
        std::string current_path = parent_path.empty() ? key : parent_path + "." + key;
        rapidjson::Value& value = it->value;

        ImGui::PushID(key);

        if (value.IsInt()) {
            render_int_field(document, value, entity_id, variant_type, key, current_path);
        } else if (value.IsFloat() || value.IsDouble()) {
            render_float_field(document, value, entity_id, variant_type, key, current_path);
        } else if (value.IsBool()) {
            render_bool_field(value, entity_id, variant_type, key, current_path);
        } else if (value.IsString()) {
            render_string_field(document, value, entity_id, variant_type, key, current_path);
        } else if (value.IsObject()) {
            if (ImGui::TreeNode(key)) {
                render_object(document, value, entity_id, variant_type, current_path);
                ImGui::TreePop();
            }
        } else if (value.IsArray()) {
            render_array_field(document, value, entity_id, variant_type, key, current_path);
        }

        ImGui::PopID();
    }
}

void Inspector::render_int_field(rapidjson::Document& document, rapidjson::Value& value,
                                  uint64_t entity_id, const std::string& variant_type,
                                  const std::string& key, const std::string& current_path) {
    std::string unique_id = std::to_string(entity_id) + "_" + variant_type + "_" + current_path;

    int int_value = value.GetInt();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", key.c_str());
    ImGui::SameLine(150.0f);

    ImGui::PushItemWidth(-1);
    bool edited = ImGui::DragInt(("##" + unique_id).c_str(), &int_value);
    ImGui::PopItemWidth();

    if (edited) {
        value.SetInt(int_value);
        m_editing_field[unique_id] = true;
    }

    if (m_editing_field[unique_id] && ImGui::IsItemDeactivatedAfterEdit()) {
        notify_engine_entity_property_changed(entity_id, variant_type, "int", current_path, std::to_string(int_value));
        m_editing_field[unique_id] = false;
    }
}

void Inspector::render_float_field(rapidjson::Document& document, rapidjson::Value& value,
                                    uint64_t entity_id, const std::string& variant_type,
                                    const std::string& key, const std::string& current_path) {
    std::string unique_id = std::to_string(entity_id) + "_" + variant_type + "_" + current_path;

    float float_value = value.IsDouble() ? static_cast<float>(value.GetDouble()) : value.GetFloat();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", key.c_str());
    ImGui::SameLine(150.0f);

    ImGui::PushItemWidth(-1);
    bool edited = ImGui::DragFloat(("##" + unique_id).c_str(), &float_value, 0.1f, 0.0f, 0.0f, "%.3f");
    ImGui::PopItemWidth();

    if (edited) {
        value.SetFloat(float_value);
        m_editing_field[unique_id] = true;
    }

    if (m_editing_field[unique_id] && ImGui::IsItemDeactivatedAfterEdit()) {
        notify_engine_entity_property_changed(entity_id, variant_type, "float", current_path, std::to_string(float_value));
        m_editing_field[unique_id] = false;
    }
}

void Inspector::render_bool_field(rapidjson::Value& value, uint64_t entity_id,
                                   const std::string& variant_type, const std::string& key,
                                   const std::string& current_path) {
    std::string unique_id = std::to_string(entity_id) + "_" + variant_type + "_" + current_path;

    bool bool_value = value.GetBool();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", key.c_str());
    ImGui::SameLine(150.0f);

    if (ImGui::Checkbox(("##" + unique_id).c_str(), &bool_value)) {
        value.SetBool(bool_value);
        notify_engine_entity_property_changed(entity_id, variant_type, "bool", current_path, bool_value ? "true" : "false");
    }
}

void Inspector::render_string_field(rapidjson::Document& document, rapidjson::Value& value,
                                     uint64_t entity_id, const std::string& variant_type,
                                     const std::string& key, const std::string& current_path) {
    std::string unique_id = std::to_string(entity_id) + "_" + variant_type + "_" + current_path;

    char buffer[256];
    strncpy(buffer, value.GetString(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", key.c_str());
    ImGui::SameLine(150.0f);

    ImGui::PushItemWidth(-1);
    bool edited = ImGui::InputText(("##" + unique_id).c_str(), buffer, sizeof(buffer));
    ImGui::PopItemWidth();

    if (edited) {
        value.SetString(buffer, document.GetAllocator());
        m_editing_field[unique_id] = true;
    }

    if (m_editing_field[unique_id] && ImGui::IsItemDeactivatedAfterEdit()) {
        notify_engine_entity_property_changed(entity_id, variant_type, "string", current_path, buffer);
        m_editing_field[unique_id] = false;
    }
}

void Inspector::render_array_field(rapidjson::Document& document, rapidjson::Value& value,
                                    uint64_t entity_id, const std::string& variant_type,
                                    const std::string& key, const std::string& current_path) {
    std::string header_label = key + " [" + std::to_string(value.Size()) + "]";

    if (ImGui::TreeNode(header_label.c_str())) {
        for (rapidjson::SizeType i = 0; i < value.Size(); ++i) {
            ImGui::PushID(static_cast<int>(i));

            std::string item_path = current_path + "[" + std::to_string(i) + "]";
            std::string item_label = "[" + std::to_string(i) + "]";
            rapidjson::Value& item = value[i];

            if (item.IsInt()) {
                render_int_field(document, item, entity_id, variant_type, item_label, item_path);
            } else if (item.IsFloat() || item.IsDouble()) {
                render_float_field(document, item, entity_id, variant_type, item_label, item_path);
            } else if (item.IsBool()) {
                render_bool_field(item, entity_id, variant_type, item_label, item_path);
            } else if (item.IsString()) {
                render_string_field(document, item, entity_id, variant_type, item_label, item_path);
            } else if (item.IsObject()) {
                if (ImGui::TreeNode(item_label.c_str())) {
                    render_object(document, item, entity_id, variant_type, item_path);
                    ImGui::TreePop();
                }
            }

            ImGui::PopID();
        }
        ImGui::TreePop();
    }
}

void Inspector::render_add_component_button(EntityDocument& entity, uint64_t entity_id) {
    float button_width = ImGui::GetContentRegionAvail().x;

    if (ImGui::Button("Add Component", ImVec2(button_width, 0))) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup")) {
        static char search_buffer[128] = "";
        ImGui::InputTextWithHint("##search", "Search...", search_buffer, sizeof(search_buffer));

        ImGui::Separator();

        std::string search_lower = search_buffer;
        std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);

        for (const auto& variant : m_variants) {
            if (variant.is_dead() || variant.get_name().empty()) continue;

            const std::string& name = variant.get_name();

            // Filter by search
            if (!search_lower.empty()) {
                std::string name_lower = name;
                std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
                if (name_lower.find(search_lower) == std::string::npos) continue;
            }

            // Check if already exists
            bool exists = false;
            if (entity.get_document().HasMember("variants")) {
                for (const auto& v : entity.get_document()["variants"].GetArray()) {
                    if (std::string(v["type"].GetString()) == name) {
                        exists = true;
                        break;
                    }
                }
            }

            if (exists) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_Disabled);
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Selectable(name.c_str())) {
                    add_variant_to_entity(entity, const_cast<VariantDocument&>(variant));
                    search_buffer[0] = '\0';
                }
            }
        }

        ImGui::EndPopup();
    }
}

void Inspector::add_variant_to_entity(EntityDocument& entity, VariantDocument& variant) {
    rapidjson::Document& entity_doc = entity.get_document();
    const rapidjson::Document& variant_doc = variant.get_document();

    uint64_t entity_id = entity_doc["entity_id"].GetUint64();

    if (!entity_doc.HasMember("variants")) {
        rapidjson::Value variants_array(rapidjson::kArrayType);
        entity_doc.AddMember("variants", variants_array, entity_doc.GetAllocator());
    }

    rapidjson::Value& variants = entity_doc["variants"];

    const std::string& type = variant_doc["type"].GetString();

    // Check duplicates
    for (const auto& v : variants.GetArray()) {
        if (std::string(v["type"].GetString()) == type) {
            return;
        }
    }

    rapidjson::Value copied(variant_doc, entity_doc.GetAllocator());
    variants.PushBack(copied, entity_doc.GetAllocator());

    notify_engine_entity_variant_added(entity_id, type);
}
