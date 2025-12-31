module;

#include "imgui.h"
#include <string>

module zeytin.console;

ImVec4 ConsoleWindow::get_log_level_color(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE:   return ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
        case LogLevel::INFO:    return ImVec4(0.5f, 0.8f, 1.0f, 1.0f);
        case LogLevel::WARNING: return ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
        case LogLevel::ERROR:   return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
        default:                return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void ConsoleWindow::render() {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

    if (ImGui::Begin("Console", nullptr, flags)) {
        if (ImGui::Button("Clear")) {
            Logger::get().clear();
        }

        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_auto_scroll);

        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        ImGui::Text("Source:");
        ImGui::SameLine();
        ImGui::Checkbox("Editor", &m_show_editor_logs);
        ImGui::SameLine();
        ImGui::Checkbox("Engine", &m_show_engine_logs);

        ImGui::SameLine();
        float line_height = ImGui::GetTextLineHeight();
        ImVec2 cursor_pos = ImGui::GetCursorPos();
        ImGui::GetWindowDrawList()->AddLine(
            ImGui::GetCursorScreenPos(),
            ImVec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + line_height),
            ImGui::GetColorU32(ImGuiCol_Separator)
        );
        ImGui::SetCursorPos(ImVec2(cursor_pos.x + 8, cursor_pos.y)); 

        ImGui::SameLine();
        ImGui::Text("Level:");

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, get_log_level_color(LogLevel::TRACE));
        ImGui::Checkbox("Trace", &m_show_trace);
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, get_log_level_color(LogLevel::INFO));
        ImGui::Checkbox("Info", &m_show_info);
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, get_log_level_color(LogLevel::WARNING));
        ImGui::Checkbox("Warning", &m_show_warning);
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, get_log_level_color(LogLevel::ERROR));
        ImGui::Checkbox("Error", &m_show_error);
        ImGui::PopStyleColor();

        ImGui::Separator();

        const float footer_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing() + 6.0f;
        
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height), false, ImGuiWindowFlags_HorizontalScrollbar);

        const auto& logs = Logger::get().get_log_messages();
        
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); 

        ImVec4 engine_bg_color(0.15f, 0.15f, 0.20f, 0.60f);
        
        float indent_width = 8.0f;
        float source_width = 80.0f;
        float level_width = 70.0f;
        float text_start_pos = indent_width + source_width + level_width;

        for (const auto& log_pair : logs) {
            LogLevel level = log_pair.first;
            const std::string& message = log_pair.second;

            bool is_engine_log = message.find("[ENGINE]") != std::string::npos;
            
            if ((level == LogLevel::TRACE && !m_show_trace) ||
                (level == LogLevel::INFO && !m_show_info) ||
                (level == LogLevel::WARNING && !m_show_warning) ||
                (level == LogLevel::ERROR && !m_show_error) ||
                (is_engine_log && !m_show_engine_logs) ||
                (!is_engine_log && !m_show_editor_logs)) {
                continue;
            }

            if (is_engine_log) {
                ImVec2 bg_min = ImGui::GetCursorScreenPos();
                float text_height = ImGui::GetTextLineHeight();
                ImVec2 bg_max = ImVec2(bg_min.x + ImGui::GetContentRegionAvail().x, bg_min.y + text_height);
                ImGui::GetWindowDrawList()->AddRectFilled(bg_min, bg_max, ImGui::ColorConvertFloat4ToU32(engine_bg_color));
            }

            ImVec2 cursor_pos = ImGui::GetCursorPos();
            
            ImGui::PushStyleColor(ImGuiCol_Text, get_log_level_color(level));
            
            ImGui::SetCursorPosX(cursor_pos.x + indent_width);
            std::string source = is_engine_log ? "[ENGINE]" : "[EDITOR]";
            ImGui::Text("%s", source.c_str());
            
            ImGui::SameLine();
            ImGui::SetCursorPosX(cursor_pos.x + indent_width + source_width);
            std::string level_text;
            switch (level) {
                case LogLevel::TRACE:   level_text = "[TRACE]"; break;
                case LogLevel::INFO:    level_text = "[INFO]"; break;
                case LogLevel::WARNING: level_text = "[WARN]"; break;
                case LogLevel::ERROR:   level_text = "[ERROR]"; break;
                default:                level_text = "[---]"; break;
            }
            ImGui::Text("%s", level_text.c_str());
            
            ImGui::SameLine();
            ImGui::SetCursorPosX(cursor_pos.x + text_start_pos);
            
            std::string display_message = message;
            
            if (is_engine_log) {
                size_t engine_prefix = display_message.find("[ENGINE]");
                if (engine_prefix != std::string::npos) {
                    display_message = display_message.substr(engine_prefix + 9); // 9 is length of "[ENGINE] "
                }
            } else {
                size_t editor_prefix = display_message.find("[EDITOR]");
                if (editor_prefix != std::string::npos) {
                    display_message = display_message.substr(editor_prefix + 9);
                }
            }
            
            const char* level_prefixes[] = {"[TRACE]", "[INFO]", "[WARN]", "[WARNING]", "[ERROR]"};
            for (const char* prefix : level_prefixes) {
                size_t prefix_pos = display_message.find(prefix);
                if (prefix_pos != std::string::npos) {
                    display_message = display_message.substr(prefix_pos + strlen(prefix) + 1); // +1 for space after prefix
                    break;
                }
            }
            
            size_t first_non_space = display_message.find_first_not_of(" \t\n\r");
            if (first_non_space != std::string::npos) {
                display_message = display_message.substr(first_non_space);
            }
            
            ImGui::TextUnformatted(display_message.c_str());
            ImGui::PopStyleColor();
        }
        
        if (!logs.empty()) {
            ImGui::Dummy(ImVec2(0, 8.0f));
        }
        
        ImGui::PopStyleVar(); 

        if (m_auto_scroll) {
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20.0f) {
                ImGui::SetScrollHereY(1.0f);
            }
        }

        ImGui::EndChild();

        ImGui::Separator();
        bool reclaim_focus = false;

        if (ImGui::InputText("Command", m_command_buffer, IM_ARRAYSIZE(m_command_buffer),
                            ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (m_command_buffer[0] != '\0') {
                //log_info() << "Command: " << m_command_buffer;
                m_command_buffer[0] = '\0';
                reclaim_focus = true;
            }
        }

        ImGui::SetItemDefaultFocus();
        if (reclaim_focus) {
            ImGui::SetKeyboardFocusHere(-1);
        }

        ImGui::End();
    }
}
