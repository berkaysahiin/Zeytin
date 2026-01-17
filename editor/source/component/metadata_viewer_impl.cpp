module;

#include "imgui.h"
#include <string>
#include <vector>

module zeytin.metadata.viewer;
import zeytin.variant.metadata;
import zeytin.variant.list;
import zeytin.entity.registry;

MetadataViewer::MetadataViewer() {}

MetadataViewer::~MetadataViewer() {}

void MetadataViewer::render() {
    ImGui::BeginChild("VariantListPane", ImVec2(250, 0), true);
    render_variant_list();
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    ImGui::BeginChild("DetailsPane", ImVec2(0, 0), true);
    render_variant_details();
    ImGui::EndChild();
}

void MetadataViewer::render_variant_list() {
    ImGui::Text("Components");
    ImGui::Separator();

    const auto variant_list_opt = EntityRegistry::get().get_variant_list();
    if (!variant_list_opt.has_value()) {
        ImGui::TextDisabled("No component list available");
        return;
    }
    
    const auto& variant_list = variant_list_opt.value().get();
    const auto& variants = variant_list.get_variants();
    
    for (int i = 0; i < variants.size(); i++) {
        const auto& variant = variants[i];
        if (variant.is_dead()) {
            continue;
        }

        const bool is_selected = (m_selected_variant_index == i);
        if (ImGui::Selectable(variant.get_name().c_str(), is_selected)) {
            m_selected_variant_index = i;
        }
    }
}

void MetadataViewer::render_variant_details() {
    auto variant_list_opt = EntityRegistry::get().get_variant_list();
    if (!variant_list_opt.has_value()) {
        ImGui::TextDisabled("No variant list available");
        return;
    }
    
    auto& variant_list = variant_list_opt.value().get();
    const auto& variants = variant_list.get_variants();
    
    if (m_selected_variant_index < 0 || m_selected_variant_index >= variants.size()) {
        ImGui::TextDisabled("Select a component to view metadata");
        return;
    }

    const auto& variant = variants[m_selected_variant_index];
    if (variant.is_dead()) {
        ImGui::TextDisabled("Selected component was removed");
        return;
    }

    const std::string& variant_name = variant.get_name();
    
    ImGui::Text("Component: %s", variant_name.c_str());
    ImGui::Separator();
    
    // get all the properties from the variant document
    const auto& doc = variant.get_document();
    
    if (!doc.HasMember("value") || !doc["value"].IsObject()) {
        ImGui::TextDisabled("No properties found");
        return;
    }
    
    const auto& value_obj = doc["value"];
    
    // display properties in a table
    if (ImGui::BeginTable("metadata_table", 3, 
        ImGuiTableFlags_Borders | 
        ImGuiTableFlags_RowBg | 
        ImGuiTableFlags_Resizable | 
        ImGuiTableFlags_ScrollY)) {
        
        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Default Value", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Annotations", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        
        // iter through properties
        for (auto it = value_obj.MemberBegin(); it != value_obj.MemberEnd(); ++it) {
            const std::string property_name = it->name.GetString();
            
            ImGui::TableNextRow();
            
            // name
            ImGui::TableNextColumn();
            ImGui::Text("%s", property_name.c_str());
            
            // default value from document
            ImGui::TableNextColumn();
            std::string value_str;
            if (it->value.IsBool()) {
                value_str = it->value.GetBool() ? "true" : "false";
            } else if (it->value.IsInt()) {
                value_str = std::to_string(it->value.GetInt());
            } else if (it->value.IsDouble()) {
                value_str = std::to_string(it->value.GetDouble());
            } else if (it->value.IsString()) {
                value_str = "\"" + std::string(it->value.GetString()) + "\"";
            } else {
                value_str = "(complex type)";
            }
            ImGui::TextWrapped("%s", value_str.c_str());
            
            // annotations
            ImGui::TableNextColumn();
            
            // get all annotations for this property from VariantMetadata
            auto annotations = VariantMetadata::get().get_all_annotations(variant_name, property_name);
            
            if (annotations.empty()) {
                ImGui::TextDisabled("(no annotations)");
            } else {
                std::string annotations_display;
                
                for (const auto& [ann_key, ann_value] : annotations) {
                    if (!annotations_display.empty()) {
                        annotations_display += ", ";
                    }
                    
                    annotations_display += ann_key;
                    
                    // show value if not "true" (default for boolean flags)
                    if (ann_value != "true") {
                        annotations_display += "=" + ann_value;
                    }
                }
                
                ImGui::TextWrapped("%s", annotations_display.c_str());
            }
        }

        ImGui::EndTable();
    }
}
