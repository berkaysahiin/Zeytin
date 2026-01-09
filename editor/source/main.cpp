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
import zeytin.engine_view;
import zeytin.variant.list;
import zeytin.variant.metadata;
import zeytin.metadata.viewer;
import zeytin.entity.list;
import zeytin.entity.registry;
import zeytin.windows.level;
import zeytin.inspector;
import zeytin.command.keyboardlistener;
import zeytin.command_history;

int main(int argc, char* argv[])
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
        EngineCommunication engine_communication;

        EntityList entity_list;
        EntityRegistry::get().set_entity_list(entity_list);  

        VariantList variant_list;
        EntityRegistry::get().set_variant_list(variant_list);
        
        // Load variant metadata (annotations)
        VariantMetadata::get().load_from_component_files("../shared_resources/components");

        Hierarchy hierarchy(variant_list.get_variants(), &entity_list);
        TestViewer test_viewer;
        EngineView engine_view;
        MetadataViewer metadata_viewer;
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
            
        window_manager.add_window("Test Viewer",
            [&test_viewer]() {
                test_viewer.render();
            });
        
        window_manager.add_window("Metadata Viewer",
            [&metadata_viewer]() {
                metadata_viewer.render();
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

        Inspector inspector(variant_list.get_variants());

        window_manager.add_window("Inspector",
            [&inspector]() {
                inspector.render();
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
