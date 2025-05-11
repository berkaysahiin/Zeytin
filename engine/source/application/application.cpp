#include "application/application.h"
#include "core/raylib_wrapper.h"
#include "core/zeytin.h"
#include "core/macros.h""
#include "raylib.h"

#include "config_manager/config_manager.h""
#include "remote_logger/remote_logger.h""

Application::Application() {
    init_window();
    init_engine();
}

void Application::init_window() {

#ifdef EDITOR_MODE
    SetTraceLogLevel(LOG_ERROR);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_ALWAYS_RUN);

    const int window_width = CONFIG_GET("window_width", int, 1280);
    const int window_height = CONFIG_GET("window_height", int, 720);

    const int window_x = CONFIG_GET("window_x", int, -1);
    const int window_y = CONFIG_GET("window_y", int, -1);

    InitWindow(window_width, window_height, "Zeytin");

    if(window_x != window_y != -1) {
        SetWindowPosition(window_x, window_y);
    }

#else
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    const int window_width = GetScreenWidth();
    const int window_height = GetScreenHeight();
    InitWindow(window_width, window_height, "Zeytin Game");
#endif

    //int monitor_refresh_rate = GetMonitorRefreshRate(GetCurrentMonitor());
    //set_target_fps(monitor_refresh_rate);
    set_target_fps(144);
    set_exit_key(0);
}

void Application::init_engine() {
    CONSTRUCT_SINGLETON(Zeytin);
}

void Application::run_frame() {
    Zeytin::get().run_frame();
}

bool Application::should_shutdown() {
    return Zeytin::get().should_die();
}

void Application::shutdown() {
}
