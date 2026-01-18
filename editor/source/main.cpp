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
import zeytin.engine_view;
import zeytin.component.registry;
import zeytin.variant.metadata;
import zeytin.component.view;
import zeytin.entity.list;
import zeytin.entity.registry;
import zeytin.windows.level;
import zeytin.windows.property_tracker;
import zeytin.inspector;
import zeytin.command.keyboardlistener;
import zeytin.command_history;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_ALWAYS_RUN);
    
    InitWindow(1280, 720, "ZeytinEditor");
    
    SetTargetFPS(144);
    SetExitKey(0);
    
    rlImGuiSetup(true);

	LoadEditorFonts();
    SetEditorTheme();

    // wrap everything in a scope to ensure proper destruction order
    {
        EngineControls engine_controls;
        EngineCommunication::get();

        EntityList entity_list;
        EntityRegistry::get().set_entity_list(entity_list);  

        initialize_component_registry();
        
        // Load variant metadata (annotations)
        VariantMetadata::get().load_from_component_files("../shared_resources/components");

        Hierarchy hierarchy(&entity_list);
        EngineView engine_view;
        ComponentViewWindow component_view;
        CommandHistory command_history;

        LevelWindow::get().set_entity_list(&entity_list);  

        WindowManager window_manager;
        window_manager.init();
        
        window_manager.add_window("Hierarchy",
            [&hierarchy]() {
                hierarchy.update();
            }); 
        
        window_manager.add_window("Console",
            []() {
                ConsoleWindow::get().render();
            });
        
        window_manager.add_window("Asset Browser",
            []() {
                AssetBrowser::get().render();
            });
            
        window_manager.add_window("Component View",
            [&component_view]() {
                component_view.render();
            });
        
        window_manager.add_window("Engine View",
            [&engine_view]() {
                engine_view.render();
            });

        window_manager.add_main_menu_component([&engine_controls]{
                engine_controls.render();
        });

        window_manager.add_window("Levels",
            []() {
                LevelWindow::get().render();
            });

        Inspector inspector;

        window_manager.add_window("Inspector",
            [&inspector]() {
                inspector.render();
            });

        window_manager.add_window("Property Tracker",
            []() {
                PropertyTrackerWindow::get().render();
            });
        
        window_manager.add_window("Command History",
            [&command_history]() {
                command_history.render();
            });
        
        // Load saved window states after all windows are added
        window_manager.load_window_config();

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
    } 

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}
