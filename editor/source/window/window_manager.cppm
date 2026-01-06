module;

#include <functional>
#include <memory>
#include <string>

export module zeytin.window;

export class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    void init();
    void render();

    void add_window(const std::string& name,
                   std::function<void()> render_func,
                   bool default_open = true,
                   const std::string& menu_path = "Windows");

    void add_main_menu_component(std::function<void()> render_func);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
