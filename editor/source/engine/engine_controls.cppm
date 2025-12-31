module;

#include <string>
#include <future>

export module zeytin.engine.controls;

export class EngineControls {
public:
    EngineControls();
    ~EngineControls();

    void render();
    void start_engine();
    void kill_engine();

    enum class BuildStatus {
        None,
        Running,
        Success,
        Failed
    };

private:
    void render_engine_controls();
    void render_play_controls();
    void render_build_status();
    
    void enter_play_mode();
    void exit_play_mode();
    
    void check_build_status();
    void monitor_build();

    bool m_is_running;
    bool m_is_play_mode;
    bool m_is_paused;
    bool m_is_engine_starting;
    
    BuildStatus m_build_status;
    std::string m_build_message;
    std::string m_build_details;
    std::future<void> m_build_monitor_future;
    bool m_build_monitor_active;
};
