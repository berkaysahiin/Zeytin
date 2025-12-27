#include "level/level_window.h"
#include "imgui.h"
#include "logger.h"
#include "resource_manager/resource_manager.h"
#include <algorithm>

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
    
    // Get and sort levels alphabetically
    auto levels = m_entity_list->get_available_levels();
    std::sort(levels.begin(), levels.end(), 
        [](const Level& a, const Level& b) {
            return a.name < b.name;
        });
    
    ImGui::Text("Available Levels:");
    for (const auto& level : levels) {
        bool is_current = (level.name == current.name);
        
        if (is_current) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        }
        
        if (ImGui::Button(level.name.c_str(), ImVec2(200, 0))) {
            m_entity_list->load_level(level);
        }
        
        if (is_current) {
            ImGui::PopStyleColor();
        }
    }
    
    ImGui::Separator();
    ImGui::Text("Create New Level:");
    ImGui::InputTextWithHint("##NewLevel", "Level name", m_new_level_name, sizeof(m_new_level_name));
    
    if (ImGui::Button("Create Level", ImVec2(150, 30))) {
        if (m_new_level_name[0] != '\0') {
            auto path = ResourceManager::get().get_resource_subdir("levels") / m_new_level_name;
            std::filesystem::create_directories(path);
            m_entity_list->load_level(Level(m_new_level_name, path));
            m_new_level_name[0] = '\0';
            log_info() << "Created new level: " << m_new_level_name << std::endl;
        }
    }
}
