module;

#include <functional>
#include <vector>
#include <string>
#include "imgui.h"

export module zeytin.window;

export struct MenuInfo {
    std::string name;        
    std::string menu_path;   
    bool visible_in_menu;    
    bool default_open;      
};

export struct WindowInfo {
    MenuInfo menu_info;
    bool is_open;
    std::function<void()> render_func;
    ImGuiWindowFlags flags;
};

export class WindowManager {
public:
    WindowManager();
    ~WindowManager() = default;

    void init();
    void render();
    
    void add_window(const std::string& name, 
                   std::function<void()> render_func,
                   bool default_open = true,
                   const std::string& menu_path = "Windows",
                   bool visible_in_menu = true,
                   ImGuiWindowFlags flags = 0);

    void set_main_dockspace_id(ImGuiID id) { m_main_dockspace_id = id; }
    ImGuiID get_main_dockspace_id() const { return m_main_dockspace_id; }

    inline void add_main_menu_component(std::function<void()> render_func) {
        this->m_main_menu_components.push_back(render_func);
    }

private:
    void render_main_menu_bar();
    void create_dockspace();
    void handle_menu_item(const std::string& menu_path, const std::string& name, bool& is_open);
    std::vector<std::function<void()>> m_main_menu_components;
    std::vector<WindowInfo> m_windows;

    ImGuiID m_main_dockspace_id;
    bool m_first_layout;
    bool m_config_exists;
    std::string m_ini_filename;
};
