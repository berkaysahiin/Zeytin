module;

#include "imgui.h"

#include <optional>
#include <string>

export module zeytin.inspector.tracking;

import zeytin.command.property;
import zeytin.property.tracker;

export bool is_property_tracked(const PropertyLocation& location, const std::optional<std::string>& key_type) {
    if (!key_type.has_value()) {
        return false;
    }

    return PropertyTracker::get().is_tracked(location);
}

export void render_property_tracking_menu(const std::string& popup_id,
                                          const PropertyLocation& location,
                                          const std::optional<std::string>& key_type) {
    bool tracked_state = is_property_tracked(location, key_type);

    if (!ImGui::BeginPopupContextItem(popup_id.c_str())) {
        return;
    }

    const bool trackable = key_type.has_value();
    ImGui::BeginDisabled(!trackable);
    if (ImGui::MenuItem("Track Property", nullptr, &tracked_state)) {
        if (trackable) {
            PropertyTracker::get().toggle_tracking(location, *key_type);
        }
    }
    ImGui::EndDisabled();
    ImGui::EndPopup();
}

export void push_tracked_label_style(const bool is_tracked) {
    if (!is_tracked) {
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
}

export void pop_tracked_label_style(const bool is_tracked) {
    if (!is_tracked) {
        return;
    }

    ImGui::PopStyleColor();
}

export void push_tracked_value_style(const bool is_tracked) {
    if (!is_tracked) {
        return;
    }

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.25f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.18f, 0.35f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.2f, 0.45f, 0.2f, 1.0f));
}

export void pop_tracked_value_style(const bool is_tracked) {
    if (!is_tracked) {
        return;
    }

    ImGui::PopStyleColor(3);
}
