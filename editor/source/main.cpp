#include "raylib.h"
#include "rlImGui.h"

import zeytin.asset;
import zeytin.level;
import zeytin.level;
import zeytin.console;
import zeytin.window;
import zeytin.theme;
import zeytin.hierarchy;
import zeytin.engine.controls;
import zeytin.engine.communication;
import zeytin.testviewer;
import zeytin.variant.list;
import zeytin.entity.list;
import zeytin.entity.registry;
import zeytin.windows.level;
import zeytin.inspector;
import zeytin.command.keyboardlistener;

int main(int argc, char* argv[])
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_ALWAYS_RUN);
    
    InitWindow(1280, 720, "ZeytinEditor");
    
    SetTargetFPS(60);
    SetExitKey(0);
    
    rlImGuiSetup(true);

	LoadEditorFonts();
    SetEditorTheme();

    EngineControls engine_controls;
    EngineCommunication engine_communication;

    EntityList entity_list;
	EntityRegistry::get().set_entity_list(entity_list);  

    VariantList variant_list;

	Hierarchy hierarchy(variant_list.get_variants(), &entity_list);
    TestViewer test_viewer;

	LevelWindow::get().set_entity_list(&entity_list);  

    WindowManager window_manager;
    window_manager.init();
    
    window_manager.add_window("Hierarchy", 
        [&hierarchy]() {
            hierarchy.update();
        },
        true, 
        "Hierarchy", 
        true); 
    
    window_manager.add_window("Console", 
        []() {
            ConsoleWindow::get().render();
        },
        true,
        "Console",
        true);
    
    window_manager.add_window("Asset Browser", 
        []() {
            AssetBrowser::get().render();
        },
        true,
        "Asset Browser",
        true);
        
    window_manager.add_window("Test Viewer", 
        [&test_viewer]() {
            test_viewer.render();
        },
        false, 
        "Test Viewer", 
        true);

   	window_manager.add_main_menu_component([&engine_controls]{
            engine_controls.render();
    });

	window_manager.add_window("Levels",
        []() {
            LevelWindow::get().render();
        },
        true,  // visible by default
        "Levels",
        true);

	Inspector inspector(variant_list.get_variants());

	// Add the Inspector window registration:
	window_manager.add_window("Inspector",
    [&inspector]() {
        inspector.render();
    },
    true,
    "Inspector",
    true);

    while (!WindowShouldClose())
    {
		handle_undo_redo();

        BeginDrawing();
        ClearBackground(BLACK);

        rlImGuiBegin();

        window_manager.render();

        rlImGuiEnd();
        EndDrawing();
    }

    engine_controls.kill_engine();

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}
