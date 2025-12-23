#pragma once

#include "logger.h"
#include "imgui.h"

#include "utility/singleton.h"

class ConsoleWindow {
    MAKE_SINGLETON(ConsoleWindow);

public:    
    void render();

    void set_show_trace(bool show) { m_show_trace = show; }
    void set_show_info(bool show) { m_show_info = show; }
    void set_show_warning(bool show) { m_show_warning = show; }
    void set_show_error(bool show) { m_show_error = show; }
    
    void set_show_editor_logs(bool show) { m_show_editor_logs = show; }
    void set_show_engine_logs(bool show) { m_show_engine_logs = show; }

    void set_auto_scroll(bool auto_scroll) { m_auto_scroll = auto_scroll; }
    bool get_auto_scroll() const { return m_auto_scroll; }

private:
    ConsoleWindow() = default;

    bool m_show_trace = true;
    bool m_show_info = true;
    bool m_show_warning = true;
    bool m_show_error = true;
    
    bool m_show_editor_logs = true;
    bool m_show_engine_logs = true;

    bool m_auto_scroll = true;

    char m_command_buffer[256] = "";

    ImVec4 get_log_level_color(LogLevel level) const;
};
