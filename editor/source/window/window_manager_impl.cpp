module;

#include "imgui.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include <filesystem>
#include <unordered_map>
#include <vector>
#include <functional>
#include <fstream>

module zeytin.window;
import zeytin.resource;
import zeytin.logger;

struct WindowManager::Impl {
    struct WindowInfo {
        std::string name;
        std::string menu_path;
        std::function<void()> render_func;
        bool is_open;
    };

    std::vector<WindowInfo> windows;
    std::vector<std::function<void()>> main_menu_components;
    std::string ini_filename;
    std::filesystem::path window_config_path;
    
    void load_window_states() {
        if (!std::filesystem::exists(window_config_path)) {
            return;
        }
        
        std::ifstream file(window_config_path);
        if (!file.is_open()) {
            return;
        }
        
        std::string json_str((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
        file.close();
        
        rapidjson::Document doc;
        doc.Parse(json_str.c_str());
        
        if (doc.HasParseError() || !doc.IsObject()) {
            return;
        }
        
        for (auto& window : windows) {
            if (doc.HasMember(window.name.c_str())) {
                window.is_open = doc[window.name.c_str()].GetBool();
            }
        }
    }
    
    void save_window_states() {
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();
        
        for (const auto& window : windows) {
            rapidjson::Value key(window.name.c_str(), allocator);
            doc.AddMember(key, window.is_open, allocator);
        }
        
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        
        std::ofstream file(window_config_path);
        if (file.is_open()) {
            file << buffer.GetString();
            file.close();
        }
    }

    void create_dockspace() {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        ImGui::End();
    }

    void render_main_menu_bar() {
        if (!ImGui::BeginMainMenuBar())
            return;

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save All", "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {}
            ImGui::EndMenu();
        }

        std::unordered_map<std::string, std::vector<WindowInfo*>> menu_paths;
        for (auto& window : windows) {
            menu_paths[window.menu_path].push_back(&window);
        }

        for (auto& [menu_path, items] : menu_paths) {
            if (ImGui::BeginMenu(menu_path.c_str())) {
                for (auto* window : items) {
                    ImGui::MenuItem(window->name.c_str(), nullptr, &window->is_open);
                }
                ImGui::EndMenu();
            }
        }

        float menu_bar_width = ImGui::GetWindowWidth();
        float current_x = ImGui::GetCursorPosX();

        for (auto& component : main_menu_components) {
            float component_width = 300.0f;
            float center_x = (menu_bar_width + current_x - component_width) / 2.0f;
            ImGui::SetCursorPosX(center_x);
            if (component) {
                component();
            }
        }

        ImGui::EndMainMenuBar();
    }
};

WindowManager::WindowManager() : m_impl(std::make_unique<Impl>()) {}

WindowManager::~WindowManager() {
    // Save window states
    m_impl->save_window_states();
    
    // Disable ImGui ini saving before our ini_filename string is destroyed
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
}

void WindowManager::init() {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    std::filesystem::path config_path = ResourceManager::get().get_editor_path();
    std::filesystem::path imgui_config_path = config_path / "imgui.ini";

    m_impl->ini_filename = imgui_config_path.string();
    m_impl->window_config_path = config_path / "windows.json";
    
    io.IniFilename = m_impl->ini_filename.c_str();
    io.IniSavingRate = 5.0f;
}

void WindowManager::load_window_config() {
    m_impl->load_window_states();
}

void WindowManager::render() {
    m_impl->create_dockspace();
    m_impl->render_main_menu_bar();

    for (auto& window : m_impl->windows) {
        if (!window.is_open)
            continue;
            
        if (ImGui::Begin(window.name.c_str(), &window.is_open)) {
            if (window.render_func) {
                window.render_func();
            }
        }
        ImGui::End();
    }
}

void WindowManager::add_window(const std::string& name,
                               std::function<void()> render_func,
                               const std::string& menu_path) {
    // Check if window state exists in imgui.ini, otherwise default to true
    // ImGui will handle the actual state, we just need a variable for the checkbox
    m_impl->windows.push_back({
        .name = name,
        .menu_path = menu_path,
        .render_func = render_func,
        .is_open = true  // Just a placeholder, ImGui::Begin controls actual visibility
    });
}

void WindowManager::add_main_menu_component(std::function<void()> render_func) {
    m_impl->main_menu_components.push_back(render_func);
}
