module;

#include "imgui.h"
#include "raylib.h"

export module zeytin.command.keyboardlistener;
import zeytin.command.manager;

export void handle_undo_redo()
{
    const bool CTRL =
        IsKeyDown(KEY_LEFT_CONTROL) ||
        IsKeyDown(KEY_RIGHT_CONTROL);

    if (ImGui::GetCurrentContext()) {
        const ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) {
            return;
        }
    }

    if (CTRL && IsKeyPressed(KEY_Z)) {
        CommandManager::get().undo();
    }

    if (CTRL && IsKeyPressed(KEY_R)) {
        CommandManager::get().redo();
    }
}
