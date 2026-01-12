module;

#include "imgui.h"

#include <string>
#include <vector>

module zeytin.windows.property_tracker;

import zeytin.selection;
import zeytin.entity.registry;
import zeytin.entity.list;
import zeytin.property.tracker;
import zeytin.engine.event;

PropertyTrackerWindow::PropertyTrackerWindow() {
    EngineEventBus::get().subscribe<bool>(
        EngineEvent::EngineStarted,
        [this](bool) {
            m_engine_running = true;
        }
    );

    EngineEventBus::get().subscribe<bool>(
        EngineEvent::EngineStopped,
        [this](bool) {
            m_engine_running = false;
        }
    );
}

PropertyTrackerWindow::~PropertyTrackerWindow() = default;

void PropertyTrackerWindow::render() {
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Property Tracker");
    ImGui::Separator();

    ImGui::Text("Engine: %s", m_engine_running ? "Running" : "Stopped");
    ImGui::Separator();

    auto entity_list_opt = EntityRegistry::get().get_entity_list();
    if (entity_list_opt) {
        auto& entity_list = entity_list_opt->get();
        ImGui::Text("Entities: %zu", entity_list.get_entities().size());
        ImGui::Text("Level: %s", entity_list.get_current_level().name.c_str());
    } else {
        ImGui::TextDisabled("Entity list unavailable");
    }

    auto selected_entity = SelectionManager::get().get_selected_entity();
    if (selected_entity) {
        ImGui::Text("Selected Entity: %llu", static_cast<unsigned long long>(*selected_entity));

        const std::size_t tracked_count = PropertyTracker::get().get_tracked_count(*selected_entity);
        ImGui::Text("Tracked Properties: %zu", tracked_count);

        const auto tracked_keys = PropertyTracker::get().get_tracked_keys(*selected_entity);
        if (!tracked_keys.empty()) {
            ImGui::BeginChild("TrackedProps", ImVec2(0, 120.0f), true);
            for (const auto& key : tracked_keys) {
                ImGui::Text("%s", key.c_str());
            }
            ImGui::EndChild();
        }
    } else {
        ImGui::Text("Selected Entity: None");
    }
}
