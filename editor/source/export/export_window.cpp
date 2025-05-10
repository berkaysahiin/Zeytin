#include "export/export_window.h"
#include "imgui.h"
#include "logger.h"
#include "resource_manager/resource_manager.h"

#include <filesystem>
#include <fstream>

void ExportWindow::render() {
    if (!m_entity_list) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Entity list not initialized");
        return;
    }

    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Export Options");
    ImGui::Separator();

    ImGui::Text("Export scene to file:");
    
    ImGui::InputTextWithHint("##SceneName", "Scene name", m_scene_name, sizeof(m_scene_name));
    
    ImGui::SameLine();
    ImGui::TextDisabled(".scene");
    
    ImGui::Spacing();
    
    if (ImGui::Button("Export Scene", ImVec2(150, 30))) {
        export_scene();
    }
    
    ImGui::SameLine();
    ImGui::TextDisabled("(Exports to shared_resources/scenes/)");

    if (m_show_success_popup) {
        ImGui::OpenPopup("Export Success");
        if (ImGui::BeginPopupModal("Export Success", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", m_success_message.c_str());
            ImGui::Separator();
            
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                m_show_success_popup = false;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
    }

    if (m_show_error_popup) {
        ImGui::OpenPopup("Export Error");
        if (ImGui::BeginPopupModal("Export Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", m_error_message.c_str());
            ImGui::Separator();
            
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                m_show_error_popup = false;
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();
        }
    }
}

void ExportWindow::export_scene() {
    if (m_scene_name[0] == '\0') {
        m_error_message = "Please enter a scene name";
        m_show_error_popup = true;
        return;
    }
    
    std::string scene_data = m_entity_list->as_string();
    
    if (scene_data.empty()) {
        m_error_message = "Failed to serialize scene (empty scene or serialization error)";
        m_show_error_popup = true;
        return;
    }
    
    std::filesystem::path scenes_path = ResourceManager::get().get_resource_subdir("scenes");
    
    std::string filename = m_scene_name;
    if (filename.find(".scene") == std::string::npos) {
        filename += ".scene";
    }
    
    std::filesystem::path scene_file_path = scenes_path / filename;
    
    try {
        std::ofstream file(scene_file_path);
        if (!file.is_open()) {
            m_error_message = "Failed to open file for writing: " + scene_file_path.string();
            m_show_error_popup = true;
            return;
        }
        
        file << scene_data;
        file.close();
        
        if (file.fail()) {
            m_error_message = "Failed to write to file: " + scene_file_path.string();
            m_show_error_popup = true;
            return;
        }
        
        std::string overwrite_info = std::filesystem::exists(scene_file_path) ? " (overwritten)" : "";
        
        log_info() << "Scene exported successfully to: " << scene_file_path << overwrite_info << std::endl;
        m_success_message = "Scene exported successfully to:\n" + scene_file_path.string() + overwrite_info;
        m_show_success_popup = true;
        
        m_scene_name[0] = '\0';
    } catch (const std::exception& e) {
        m_error_message = "Error exporting scene: " + std::string(e.what());
        m_show_error_popup = true;
    }
}
