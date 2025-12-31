module;

#include "imgui_test_engine/imgui_te_engine.h"
#include "imgui_test_engine/imgui_te_context.h"
#include "imgui_test_engine/imgui_te_ui.h"

export module zeytin.testmanager;

export class TestManager {
public:
    TestManager();
    ~TestManager();

    void update();
    void shutdown();
    void post_swap();
    
private:
    void register_all_tests();
    bool m_window_visible = true;

    ImGuiTestEngine* m_test_engine = nullptr;
};
