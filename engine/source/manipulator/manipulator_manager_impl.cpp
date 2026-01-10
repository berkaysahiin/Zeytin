module;

#include <memory>
#include <unordered_map>
#include "raylib.h"

module zeytin.manipulator.manager;
import zeytin.manipulator;
import zeytin.manipulator.translate;
import zeytin.manipulator.rotate;
import zeytin.manipulator.scale;
import zeytin.game.transform;
import zeytin.raylib;
import zeytin.logger;
import zeytin.zeytin;
import zeytin.query;

struct ManipulatorManager::Impl {
    std::unordered_map<ManipulatorType, std::unique_ptr<IManipulator>> manipulators;
    ManipulatorType active_type = ManipulatorType::Translate;
    bool initialized = false;
};

ManipulatorManager::ManipulatorManager()
    : m_impl(std::make_unique<Impl>()) {
}

ManipulatorManager::~ManipulatorManager() = default;

void ManipulatorManager::initialize() {
    if (m_impl->initialized) {
        log_warning("ManipulatorManager already initialized");
        return;
    }

    register_manipulator(ManipulatorType::Translate, std::make_unique<TranslateManipulator>());
    register_manipulator(ManipulatorType::Rotate, std::make_unique<RotateManipulator>());
    register_manipulator(ManipulatorType::Scale, std::make_unique<ScaleManipulator>());

    set_active(ManipulatorType::Translate);
    m_impl->initialized = true;
    log_info("ManipulatorManager initialized");
}

void ManipulatorManager::register_manipulator(ManipulatorType type, std::unique_ptr<IManipulator> manipulator) {
    if (!manipulator) {
        log_error("Attempted to register null manipulator");
        return;
    }

    if (m_impl->manipulators.find(type) != m_impl->manipulators.end()) {
        log_warning("Manipulator already registered, replacing");
    }

    const auto name = manipulator->get_name();
    m_impl->manipulators[type] = std::move(manipulator);
    log_info("Registered manipulator: {}", name);
}

void ManipulatorManager::set_active(ManipulatorType type) {
    if (m_impl->manipulators.find(type) == m_impl->manipulators.end()) {
        log_error("Cannot set active manipulator: type not found");
        return;
    }

    // reset previous manipulator if different
    if (m_impl->active_type != type) {
        auto prev_it = m_impl->manipulators.find(m_impl->active_type);
        if (prev_it != m_impl->manipulators.end()) {
            prev_it->second->reset();
        }
    }

    m_impl->active_type = type;
    log_info("Active manipulator set to: {}", m_impl->manipulators[type]->get_name());
}

ManipulatorType ManipulatorManager::get_active_type() const {
    return m_impl->active_type;
}

void ManipulatorManager::handle_keyboard_shortcuts() {
    if (is_key_pressed(KEY_W)) {
        set_active(ManipulatorType::Translate);
    } else if (is_key_pressed(KEY_E)) {
        set_active(ManipulatorType::Rotate);
    } else if (is_key_pressed(KEY_R)) {
        set_active(ManipulatorType::Scale);
    }
}

void ManipulatorManager::handle_selected(const EntityID selected_entity_id) {
	auto ctransform_opt = Query::try_get<CTransform>(selected_entity_id);

	if(!ctransform_opt) return;

	Context ctx {
		.entity_id = selected_entity_id,
		.camera = Zeytin::get().get_camera(),
		.is_play_mode = Zeytin::get().is_play_mode(),
		.transform = Query::get<CTransform>(selected_entity_id)
	};

    auto it = m_impl->manipulators.find(m_impl->active_type);
    if (it != m_impl->manipulators.end()) {
        it->second->update(ctx);
    }
}

bool ManipulatorManager::is_any_active() const {
    auto it = m_impl->manipulators.find(m_impl->active_type);
    if (it != m_impl->manipulators.end()) {
        return it->second->is_active();
    }
    return false;
}
