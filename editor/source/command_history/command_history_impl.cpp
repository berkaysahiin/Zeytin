module;

#include "imgui.h"
#include <memory>

module zeytin.command_history;
import zeytin.command.manager;

struct CommandHistory::Impl {
    bool window_open = true;
};

CommandHistory::CommandHistory()
    : pImpl(std::make_unique<Impl>()) {
}

CommandHistory::~CommandHistory() = default;

void CommandHistory::render() {
    if (!pImpl->window_open) {
        return;
    }

    if (ImGui::Begin("Command History", &pImpl->window_open)) {
        CommandManager& cmd_manager = CommandManager::get();
        
        size_t history_size = cmd_manager.get_history_size();
        size_t current_pos = cmd_manager.get_current_position();
        
        // show stats
        ImGui::Text("Total Commands: %zu", history_size);
        ImGui::Text("Current Position: %zu", current_pos);
        ImGui::Separator();
        
        // show undo/redo buttons
        if (ImGui::Button("Undo") && cmd_manager.can_undo()) {
            cmd_manager.undo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Redo") && cmd_manager.can_redo()) {
            cmd_manager.redo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear All")) {
            cmd_manager.clear();
        }
        
        ImGui::Separator();
        
        // command stack 
        ImGui::Text("Command Stack (newest at top):");
        ImGui::BeginChild("CommandStack", ImVec2(0, 0), true);
        
        for (int i = static_cast<int>(history_size) - 1; i >= 0; i--) {
            auto desc = cmd_manager.get_command_description(i);
            std::string display_text = desc.value_or("Unknown");
            
            if (i == static_cast<int>(current_pos) - 1 && current_pos > 0) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f)); // Green for current
                ImGui::Text("[CURRENT] %d: %s", i, display_text.c_str());
                ImGui::PopStyleColor();
            } else if (i < static_cast<int>(current_pos)) {
                ImGui::Text("[DONE] %d: %s", i, display_text.c_str());
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Gray for undone
                ImGui::Text("[UNDONE] %d: %s", i, display_text.c_str());
                ImGui::PopStyleColor();
            }
        }
        
        ImGui::EndChild();
    }
    ImGui::End();
}
