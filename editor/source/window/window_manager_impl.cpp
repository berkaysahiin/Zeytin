module;

#include "imgui.h"

#include <filesystem>
#include <unordered_map>
#include <vector>
#include <functional>

module zeytin.window;
import zeytin.resource;

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

WindowManager::~WindowManager() = default;

void WindowManager::init() {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    std::filesystem::path config_path = ResourceManager::get().get_editor_path();
    std::filesystem::path imgui_config_path = config_path / "imgui.ini";

    m_impl->ini_filename = imgui_config_path.string();
    io.IniFilename = m_impl->ini_filename.c_str();
    io.IniSavingRate = 5.0f;
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
                               bool default_open,
                               const std::string& menu_path) {
    m_impl->windows.push_back({
        .name = name,
        .menu_path = menu_path,
        .render_func = render_func,
        .is_open = default_open
    });
}

void WindowManager::add_main_menu_component(std::function<void()> render_func) {
    m_impl->main_menu_components.push_back(render_func);
}
