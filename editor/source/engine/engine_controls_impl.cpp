module;

#include "imgui.h"


#include <filesystem>
#include <fstream>
#include <sstream>
#include <future>
#include <thread>
#include <chrono>
#include <sys/wait.h>

module zeytin.engine.controls;
import zeytin.resource;
import zeytin.engine.event;
import zeytin.engine.message;
import zeytin.common.message.editor_to_engine.enter_play_mode;
import zeytin.common.message.editor_to_engine.exit_play_mode;
import zeytin.common.message.editor_to_engine.pause_play_mode;
import zeytin.common.message.editor_to_engine.unpause_play_mode;
import zeytin.common.message.editor_to_engine.window_state;
import zeytin.common.message.editor_to_engine.die;
import zeytin.logger;

EngineControls::EngineControls() 
    : m_is_running(false)
    , m_is_play_mode(false)
    , m_is_paused(false)
    , m_is_engine_starting(false)
    , m_build_status(BuildStatus::None)
    , m_build_monitor_active(false)
{
    EngineEventBus::get().subscribe<bool>(
        EngineEvent::EngineStarted,
        [this](const bool& success) {
            if (success) {
                m_is_running = true;
                m_is_engine_starting = false;
                
                {
                    std::lock_guard<std::mutex> lock(m_build_mutex);
                    m_build_status = BuildStatus::None;
                }

                if (m_engine_window_hidden) {
                    send_window_state(true);
                }
            }
        }
    );

    EngineEventBus::get().subscribe<bool>(
        EngineEvent::EngineStopped,
        [this](auto _) {
            if(m_is_play_mode) {
                exit_play_mode();
            }
            m_is_running = false;
            m_is_engine_starting = false;
        }
    );

}

EngineControls::~EngineControls() {
    m_build_monitor_active = false;
    if (m_build_monitor_future.valid()) {
        m_build_monitor_future.wait();
    }
    kill_engine();
}

void EngineControls::render() {
    render_engine_controls();
    render_play_controls();

    BuildStatus status;
    {
        std::lock_guard<std::mutex> lock(m_build_mutex);
        status = m_build_status;
    }
    
    if (status == BuildStatus::Running || status == BuildStatus::Failed) {
        render_build_status();
    }
}

void EngineControls::render_engine_controls() {
    if (ImGui::Button("Kill Engine")) {
        kill_engine();
    }

    ImGui::SameLine();

    if (m_is_running) {
        if (m_engine_window_hidden) {
            if (ImGui::Button("Show Window")) {
                m_engine_window_hidden = false;
                send_window_state(false);
            }
        } else {
            if (ImGui::Button("Hide Window")) {
                m_engine_window_hidden = true;
                send_window_state(true);
            }
        }
        ImGui::SameLine();
    }

    if (!m_is_running) {
        if (!m_is_engine_starting) {
            BuildStatus status;
            {
                std::lock_guard<std::mutex> lock(m_build_mutex);
                status = m_build_status;
            }
            
            if (status == BuildStatus::Running) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui::Button("Building...");
                ImGui::PopStyleColor(4);
            } else if (ImGui::Button("Start Engine")) {
                m_is_engine_starting = true;
                start_engine();
            }
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::Button("Starting...");
            ImGui::PopStyleColor(4);
        }
    } else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Engine Running");
    }
    
    ImGui::SameLine(ImGui::GetWindowWidth() - 120);
    
    BuildStatus status_for_display;
    {
        std::lock_guard<std::mutex> lock(m_build_mutex);
        status_for_display = m_build_status;
    }
    
    switch (status_for_display) {
        case BuildStatus::Running:
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Building...");
            break;
        case BuildStatus::Success:
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Build OK");
            break;
        case BuildStatus::Failed:
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Build Failed!");
            break;
        default:
            break;
    }
}

void EngineControls::render_play_controls() {
    if (m_is_running) {
        ImGui::PushStyleColor(ImGuiCol_Button, m_is_play_mode ? ImVec4(0.0f, 0.5f, 0.0f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_is_play_mode ? ImVec4(0.0f, 0.7f, 0.0f, 1.0f) : ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, m_is_play_mode ? ImVec4(0.0f, 0.8f, 0.0f, 1.0f) : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        
        if (ImGui::Button("Play")) {
            m_is_play_mode = !m_is_play_mode;
            m_is_play_mode ? enter_play_mode() : exit_play_mode();
        }
        
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, m_is_paused ? ImVec4(0.8f, 0.5f, 0.0f, 1.0f) : ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_is_paused ? ImVec4(0.9f, 0.6f, 0.0f, 1.0f) : ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, m_is_paused ? ImVec4(1.0f, 0.7f, 0.0f, 1.0f) : ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        
        if (ImGui::Button("Pause")) {
            m_is_paused = !m_is_paused;
            if (m_is_paused) {
                EngineEventBus::get().publish<bool>(EngineEvent::PausePlayMode, true);
                send_message_to_engine<EditorPausePlayModeMessage>();
            } else {
                EngineEventBus::get().publish<bool>(EngineEvent::UnPausePlayMode, true);
                send_message_to_engine<EditorUnpausePlayModeMessage>();
            }
        }
        
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        if (m_is_play_mode) {
            if (m_is_paused) {
                ImGui::TextColored(ImVec4(0.8f, 0.5f, 0.0f, 1.0f), "PAUSED");
            } else {
                ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "PLAYING");
            }
        }
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        
        ImGui::Button("Play");
        ImGui::SameLine();
        ImGui::Button("Pause");
        
        ImGui::PopStyleColor(4);
    }
}

void EngineControls::render_build_status() {
    BuildStatus status;
    std::string message;
    std::string details;
    
    {
        std::lock_guard<std::mutex> lock(m_build_mutex);
        status = m_build_status;
        message = m_build_message;
        details = m_build_details;
    }
    
    if (status == BuildStatus::Failed) {
        m_is_engine_starting = false;
        m_is_running = false;
        
        ImGui::OpenPopup("Build Failed");
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::BeginPopupModal("Build Failed", nullptr, ImGuiWindowFlags_None)) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Engine build failed!");
            ImGui::Separator();
            
            if (!message.empty()) {
                ImGui::TextWrapped("%s", message.c_str());
            }
            
            if (!details.empty()) {
                ImGui::Separator();
                ImGui::Text("Build output:");
                ImGui::BeginChild("ErrorDetails", ImVec2(0, -30), true);
                ImGui::TextWrapped("%s", details.c_str());
                ImGui::EndChild();
            }
            
            ImGui::Separator();
            if (ImGui::Button("Close", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                std::lock_guard<std::mutex> lock(m_build_mutex);
                m_build_status = BuildStatus::None;
                m_build_message.clear();
                m_build_details.clear();
            }
            ImGui::EndPopup();
        }
    }
    else if (status == BuildStatus::Running) {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 210, 50));
        ImGui::SetNextWindowSize(ImVec2(200, 80));
        ImGui::Begin("Building", nullptr, 
                    ImGuiWindowFlags_NoTitleBar | 
                    ImGuiWindowFlags_NoResize | 
                    ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoCollapse);
        
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Building Runtime...");
        
        float progress = (float)(ImGui::GetTime() * 0.5f);
        progress = progress - (int)progress;
        
        ImGui::ProgressBar(progress, ImVec2(-1, 0), "");
        
        ImGui::End();
    }
}

void EngineControls::start_engine() {
    log_info("Attempting to start the engine...");
    
    {
        std::lock_guard<std::mutex> lock(m_build_mutex);
        m_build_status = BuildStatus::Running;
        m_build_message.clear();
        m_build_details.clear();
    }
    
    monitor_build();
}

void EngineControls::monitor_build() {
    if (m_build_monitor_future.valid()) {
        m_build_monitor_active = false;
        m_build_monitor_future.wait();
    }
    
    m_build_monitor_active = true;
    log_info("Starting build process...");
    
    m_build_monitor_future = std::async(std::launch::async, [this]() {
        std::string engine_path = ResourceManager::get().get_engine_path().string();
        std::string build_script = engine_path + "/build_and_run.sh";
        
        std::string temp_output_file = engine_path + "/.build_output.log";
        std::string build_command = "\"" + build_script + "\" > \"" + temp_output_file + "\" 2>&1";
        
        log_info("Executing: {}", build_command);
        int result = std::system(build_command.c_str());
        
        // WIFEXITED checks if process terminated normally
        // WEXITSTATUS extracts the actual exit code
        int exit_code = WIFEXITED(result) ? WEXITSTATUS(result) : -1;
        
        if (exit_code != 0) {
            log_error("Build failed with exit code: {}", exit_code);
            
            std::string build_details_temp;
            
            // read build output from temporary file
            try {
                if (std::filesystem::exists(temp_output_file)) {
                    std::ifstream output_file(temp_output_file);
                    if (output_file.is_open()) {
                        std::stringstream buffer;
                        buffer << output_file.rdbuf();
                        build_details_temp = buffer.str();
                        output_file.close();
                        
                        if (build_details_temp.empty()) {
                            log_error("Build output file was empty");
                        }                     } else {
                    	log_error("Failed to open build output file: {}", temp_output_file);
                    }
                } else {
                    log_error("Build output file does not exist: {}", temp_output_file);
                }
            } catch (const std::exception& e) {
                log_error("Failed to read build log: {}", e.what());
            }
            
            {
                std::lock_guard<std::mutex> lock(m_build_mutex);
                m_build_status = BuildStatus::Failed;
                m_build_message = "Build failed with exit code " + std::to_string(exit_code);
                m_build_details = build_details_temp;
            }
            
            std::filesystem::remove(temp_output_file);
        } else {
            log_info("Build and launch successful!");
            
            {
                std::lock_guard<std::mutex> lock(m_build_mutex);
                m_build_status = BuildStatus::Success;
            }
            
            std::filesystem::remove(temp_output_file);
            
            // auto-hide success status after 3 seconds
            std::this_thread::sleep_for(std::chrono::seconds(3));
            
            {
                std::lock_guard<std::mutex> lock(m_build_mutex);
                m_build_status = BuildStatus::None;
            }
        }
    });
}

void EngineControls::kill_engine() {
    log_info("Killing the engine...");

    m_is_play_mode = false;
    m_is_running = false;
    m_is_paused = false;
    m_is_engine_starting = false;

    send_message_to_engine<EditorDieMessage>();
}

void EngineControls::enter_play_mode() {
    EngineEventBus::get().publish<bool>(EngineEvent::EnterPlayMode, m_is_paused);
    send_message_to_engine<EditorEnterPlayModeMessage>(m_is_paused);
}

void EngineControls::exit_play_mode() {
    m_is_play_mode = false;
    m_is_paused = false;
    EngineEventBus::get().publish<bool>(EngineEvent::ExitPlayMode, true);
    send_message_to_engine<EditorExitPlayModeMessage>();
}

void EngineControls::send_window_state(bool hidden) {
    send_message_to_engine<EditorWindowStateMessage>(hidden);
}
