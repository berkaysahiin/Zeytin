module;

#include "core/macros.h"
#include "raylib.h"

#include <string>

module zeytin.application;
import zeytin.config;
import zeytin.raylib;
import zeytin.zeytin;

struct Application::Impl 
{
	ConfigManager config;
};

Application::Application() : pImpl(new Impl()){
    init_window();
    init_engine();
}

Application::~Application()
{
	delete pImpl;
}

void Application::init_window() {

#ifdef EDITOR_MODE
    SetTraceLogLevel(LOG_ERROR);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TOPMOST | FLAG_WINDOW_ALWAYS_RUN | FLAG_WINDOW_HIDDEN);

	const int screen_width = pImpl->config.get_or("screen_width", 1280);
    const int screen_height = pImpl->config.get_or("screen_height",  720);

    const int window_x = pImpl->config.get_or("window_x", -1);
    const int window_y = pImpl->config.get_or("window_y", -1);

    InitWindow(screen_width, screen_height, "Zeytin");

    if(window_x != -1 && window_y != -1) {
        SetWindowPosition(window_x, window_y);
    }

#else
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    const int window_width = GetScreenWidth();
    const int window_height = GetScreenHeight();

	std::string ZeytinGame = "ZeytinGame";
	const std::string window_name = pImpl->config.get_or("window_name", ZeytinGame);

    InitWindow(window_width, window_height, window_name.c_str());
#endif

    init_audio_device();
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
