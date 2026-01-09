module;

#include <cstdint>

module zeytin.game.transform;
import zeytin.raylib;

#ifdef EDITOR_MODE
import zeytin.zeytin;
import zeytin.manipulator;
import zeytin.manipulator.manager;
#endif

void CTransform::on_update() {
#ifdef EDITOR_MODE
    if (!Zeytin::get().is_play_mode()) {
        // Handle keyboard shortcuts for switching manipulators
        ManipulatorManager::get().handle_keyboard_shortcuts();

        Context ctx {
            .transform = *this,
            .entity_id = get_id(),
            .camera = Zeytin::get().get_camera(),
            .is_play_mode = Zeytin::get().is_play_mode()
        };

        std::uint64_t selected_entity_id = Zeytin::get().get_selected_entity();
        ManipulatorManager::get().update_selected(selected_entity_id, ctx);
    }
#endif
}

