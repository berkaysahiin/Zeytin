module;

#include "imgui.h"
#include <algorithm>
#include <filesystem>
#include <fstream>

module zeytin.windows.level;
import zeytin.resource;

void LevelWindow::render() {
    if (!m_entity_list) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Entity list not initialized");
        return;
    }

    const Level& current = m_entity_list->get_current_level();
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Level Manager");
    ImGui::Separator();
    
    ImGui::Text("Current Level: %s", current.name.c_str());
    ImGui::Separator();
    
    auto levels = m_entity_list->get_available_levels();
    std::sort(levels.begin(), levels.end(), 
        [](const Level& a, const Level& b) {
            return a.name < b.name;
        });
    
    ImGui::Text("Available Levels:");
    
    if (ImGui::BeginTable("LevelsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Load", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Export", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();
        
        for (size_t i = 0; i < levels.size(); ++i) {
            const auto& level = levels[i];
            bool is_current = (level.name == current.name);
            
            std::string row_id = "level_row_" + level.name;
            ImGui::PushID(row_id.c_str());
            
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            
            if (is_current) {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%s", level.name.c_str());
            } else {
                ImGui::Text("%s", level.name.c_str());
            }
            
            ImGui::TableNextColumn();
            
            if (is_current) {
                ImGui::BeginDisabled();
                ImGui::Button("Loaded");
                ImGui::EndDisabled();
            } else {
                if (ImGui::Button("Load")) {
                    m_entity_list->load_level(level);
                }
            }
            
            ImGui::TableNextColumn();
            if (ImGui::Button("Export")) {
                export_level_as_scene(level);
            }
            
            ImGui::PopID();
        }
        
        ImGui::EndTable();
    }
    
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Create New Level:");
    ImGui::InputTextWithHint("##NewLevel", "Level name", m_new_level_name, sizeof(m_new_level_name));
    
    if (ImGui::Button("Create Level", ImVec2(150, 30))) {
        if (m_new_level_name[0] != '\0') {
            auto path = ResourceManager::get().get_resource_subdir("levels") / m_new_level_name;
            std::filesystem::create_directories(path);
            m_entity_list->load_level(Level(m_new_level_name, path));
            m_new_level_name[0] = '\0';
            //log_info() << "Created new level: " << m_new_level_name << std::endl;
        }
    }
    
    if (m_show_export_success) {
        ImGui::OpenPopup("LevelExportSuccess");
        if (ImGui::BeginPopupModal("LevelExportSuccess", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", m_export_message.c_str());
            ImGui::Separator();
            
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                m_show_export_success = false;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
    }

    if (m_show_export_error) {
        ImGui::OpenPopup("LevelExportError");
        if (ImGui::BeginPopupModal("LevelExportError", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", m_export_message.c_str());
            ImGui::Separator();
            
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                m_show_export_error = false;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
    }
}

void LevelWindow::export_level_as_scene(const Level& level) {
    if (!level.is_valid()) {
        m_export_message = "Invalid level";
        m_show_export_error = true;
        return;
    }
    
    Level previous_level = m_entity_list->get_current_level();
    m_entity_list->load_level(level);
    
    std::string scene_data = m_entity_list->as_string();
    
    if (scene_data.empty()) {
        m_export_message = "Failed to serialize level (empty or error)";
        m_show_export_error = true;
        // Restore previous level
        m_entity_list->load_level(previous_level);
        return;
    }
    
    std::filesystem::path scenes_path = ResourceManager::get().get_resource_subdir("scenes");
    
    std::string filename = level.name + ".scene";
    std::filesystem::path scene_file_path = scenes_path / filename;
    
    try {
        std::ofstream file(scene_file_path);
        if (!file.is_open()) {
            m_export_message = "Failed to open file for writing: " + scene_file_path.string();
            m_show_export_error = true;
            m_entity_list->load_level(previous_level);
            return;
        }
        
        file << scene_data;
        file.close();
        
        if (file.fail()) {
            m_export_message = "Failed to write to file: " + scene_file_path.string();
            m_show_export_error = true;
            m_entity_list->load_level(previous_level);
            return;
        }
        
        //log_info() << "Exported level '" << level.name << "' to: " << scene_file_path << std::endl;
        m_export_message = "Level '" + level.name + "' exported to:\n" + scene_file_path.string();
        m_show_export_success = true;
        
        m_entity_list->load_level(previous_level);
        
    } catch (const std::exception& e) {
        m_export_message = "Error exporting level: " + std::string(e.what());
        m_show_export_error = true;
        m_entity_list->load_level(previous_level);
    }
}
