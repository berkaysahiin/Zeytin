module;

#include "imgui.h"
#include "rapidjson/document.h"

#include <string>
#include <unistd.h>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <variant> // IWYU pragma: keep

module zeytin.inspector;
import zeytin.selection;
import zeytin.engine.event;
import zeytin.resource;
import zeytin.command.manager;
import zeytin.command.property;
import zeytin.entity.document;
import zeytin.command.component;  
import zeytin.logger;
import zeytin.entity.registry;
import zeytin.variant.metadata;
import zeytin.inspector.utils;
import zeytin.inspector.tracking;
import zeytin.inspector.enable_if;
import zeytin.property.tracker;
import zeytin.asset;

struct Inspector::Impl {
    std::vector<VariantDocument>& variants;
    std::map<std::string, PropertyValue> editing_original_values;

    Impl(std::vector<VariantDocument>& vars) : variants(vars) {}

    void render_entity_header(EntityDocument& entity);
    void render_variants(EntityDocument& entity);
    void render_variant(rapidjson::Document& document, rapidjson::Value& variant, 
                        int index, uint64_t entity_id);

    void render_object(rapidjson::Document& document, rapidjson::Value& object,
                       uint64_t entity_id, const std::string& variant_type,
                       const std::string& parent_path = "");

    void render_property(rapidjson::Document& document, rapidjson::Value& value,
                        uint64_t entity_id, const std::string& variant_type,
                        const std::string& key, const std::string& current_path);

    void render_array_field(rapidjson::Document& document, rapidjson::Value& value,
                            uint64_t entity_id, const std::string& variant_type,
                            const std::string& key, const std::string& current_path);

    void render_add_component_button(EntityDocument& entity, uint64_t entity_id);
};

Inspector::Inspector(std::vector<VariantDocument>& variants)
    : pImpl(std::make_unique<Impl>(variants))
{
}

Inspector::~Inspector() = default;

void Inspector::render() {
	PropertyTracker::get().update();

	EntityDocument* entity = SelectionManager::get().get_selected_entity_unsafe();

	if(entity == nullptr) {
		return;
	}

	if(entity->is_dead()) {
		log_trace("Selected entity is dead: {}", entity->get_id());
		return;
	}

    pImpl->render_entity_header(*entity);
    ImGui::Separator();
    pImpl->render_variants(*entity);
}

void Inspector::Impl::render_entity_header(EntityDocument& entity) {
    rapidjson::Document& doc = entity.get_document();
    uint64_t entity_id = 0;
    if (doc.HasMember("entity_id") && doc["entity_id"].IsUint64()) {
        entity_id = doc["entity_id"].GetUint64();
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Text("%s", entity.get_name().c_str());
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::TextDisabled("(ID: %llu)", static_cast<unsigned long long>(entity_id));
}

void Inspector::Impl::render_variants(EntityDocument& entity) {
    rapidjson::Document& doc = entity.get_document();

    uint64_t entity_id = 0;
    if (doc.HasMember("entity_id") && doc["entity_id"].IsUint64()) {
        entity_id = doc["entity_id"].GetUint64();
    }

    if (!doc.HasMember("variants") || !doc["variants"].IsArray()) {
        render_add_component_button(entity, entity_id);
        return;
    }

    rapidjson::Value& variants_array = doc["variants"];

    for (rapidjson::SizeType i = 0; i < variants_array.Size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        render_variant(doc, variants_array[i], static_cast<int>(i), entity_id);
        ImGui::PopID();
    }

    ImGui::Spacing();
    render_add_component_button(entity, entity_id);
}

void Inspector::Impl::render_variant(rapidjson::Document& document, rapidjson::Value& variant,
                                int index, uint64_t entity_id) {
    if (!variant.IsObject() || !variant.HasMember("type")) {
        return;
    }

    std::string variant_type = variant["type"].GetString();

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25f, 0.25f, 0.3f, 1.0f));

    bool is_open = ImGui::CollapsingHeader(variant_type.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    ImGui::PopStyleColor(3);

    if (ImGui::BeginPopupContextItem("variant_context")) {
        if (ImGui::MenuItem("Remove Component")) {
			auto command = std::make_unique<RemoveComponentCommand>(entity_id, variant_type);
            CommandManager::get().execute_command(std::move(command));
        }
        ImGui::EndPopup();
    }

    if (is_open) {
        ImGui::Indent(10.0f);

        render_object(document, variant, entity_id, variant_type, "");

        ImGui::Unindent(10.0f);
    }

    ImGui::Spacing();
}

void Inspector::Impl::render_object(rapidjson::Document& document, rapidjson::Value& object,
                       uint64_t entity_id, const std::string& variant_type,
                       const std::string& parent_path) {
    for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it) {
        const char* key = it->name.GetString();
        
        // Skip metadata fields
        if (std::string(key) == "type" || std::string(key) == "entity_id") {
            continue;
        }
        
        if (std::string(key) == "value" && it->value.IsObject()) {
            render_object(document, it->value, entity_id, variant_type, parent_path);
            continue;
        }
        
        std::string current_path = parent_path.empty() ? key : parent_path + "." + key;
        rapidjson::Value& value = it->value;

        ImGui::PushID(key);

        if (value.IsObject()) {
            if (ImGui::TreeNode(key)) {
                render_object(document, value, entity_id, variant_type, current_path);
                ImGui::TreePop();
            }
        } else if (value.IsArray()) {
            render_array_field(document, value, entity_id, variant_type, key, current_path);
        } else {
            render_property(document, value, entity_id, variant_type, key, current_path);
        }

        ImGui::PopID();
    }
}

void Inspector::Impl::render_property(rapidjson::Document& document, rapidjson::Value& value,
                                uint64_t entity_id, const std::string& variant_type,
                                const std::string& key, const std::string& current_path) {

	// If we should render this property at all ? 
	const bool is_hidden = VariantMetadata::get().has_annotation(variant_type, key, "HIDDEN");
	if(is_hidden) return;

	const auto enable_if = VariantMetadata::get().get_annotation(variant_type, key, "ENABLE_IF");
	if (enable_if.has_value()) {
		const auto enabled = EnableIfTracker::get().get_value(entity_id, variant_type, current_path, *enable_if);
		if (!enabled.value_or(false)) {
			return;
		}
	}

	const bool is_readonly = VariantMetadata::get().has_annotation(variant_type, key, "READONLY");
	const auto tooltip = VariantMetadata::get().get_annotation(variant_type, key, "TOOLTIP");
	const auto min_annotation = VariantMetadata::get().get_annotation(variant_type, key, "MIN");
	const auto max_annotation = VariantMetadata::get().get_annotation(variant_type, key, "MAX");
	const auto key_type = get_key_type_from_value(value);

    const std::string unique_id = std::to_string(entity_id) + "_" + variant_type + "_" + current_path;


    const PropertyLocation location{entity_id, variant_type, current_path};
    const bool is_tracked = is_property_tracked(location, key_type);

    ImGui::AlignTextToFramePadding();
    push_tracked_label_style(is_tracked);
    ImGui::Text("%s", key.c_str());
    pop_tracked_label_style(is_tracked);
    show_tooltip_if_hovered(tooltip);

    render_property_tracking_menu("track_property_context_label##" + unique_id, location, key_type);

    ImGui::SameLine(150.0f);
    ImGui::PushItemWidth(-1);


    if (value.IsInt()) {
        int int_value = value.GetInt();
        const IntBounds bounds = make_int_bounds(min_annotation, max_annotation);

        push_tracked_value_style(is_tracked);
        ImGui::BeginDisabled(is_readonly);
        bool edited = ImGui::DragInt(("##" + unique_id).c_str(), &int_value, 1.0f,
                                     bounds.use ? bounds.min : 0,
                                     bounds.use ? bounds.max : 0);
        bool activated = ImGui::IsItemActivated();
        bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
        ImGui::EndDisabled();
        render_property_tracking_menu("track_property_context_value##" + unique_id, location, key_type);
        pop_tracked_value_style(is_tracked);

        if (!is_readonly) {
            if (activated || edited || deactivated) {
                log_info("INT [{}]: activated={}, edited={}, deactivated={}, old={}, new={}", 
                         key, activated, edited, deactivated, value.GetInt(), int_value);
            }

            if (activated) {
                editing_original_values[unique_id] = value.GetInt();
                log_info("Captured old value for {}: {}", key, value.GetInt());
            }

            if (edited) {
                value.SetInt(int_value);
            }

            if (deactivated) {
                log_info("Deactivated for {}", key);
                auto it = editing_original_values.find(unique_id);
                if (it != editing_original_values.end()) {
                    int old_value = std::get<int>(it->second);

                    log_info("Found old value: {}, new value: {}", old_value, int_value);

                    if (old_value != int_value) {
                        log_info("Creating command for {} change: {} -> {}", key, old_value, int_value);
                        auto command = std::make_unique<PropertyChangeCommand>(
                            PropertyLocation{entity_id, variant_type, current_path},
                            old_value,
                            int_value
                        );
                        CommandManager::get().execute_command(std::move(command));
                    } else {
                        log_info("Values are equal, no command needed");
                    }

                    editing_original_values.erase(it);
                } else {
                    log_error("Deactivated for {} but no original value found!", key);
                }
            }
        }
    } 
    else if (value.IsFloat() || value.IsDouble()) {
        float float_value = value.IsDouble() ? 
            static_cast<float>(value.GetDouble()) : value.GetFloat();
        const FloatBounds bounds = make_float_bounds(min_annotation, max_annotation);

        push_tracked_value_style(is_tracked);
        ImGui::BeginDisabled(is_readonly);
        bool edited = ImGui::DragFloat(("##" + unique_id).c_str(), &float_value, 0.1f,
                                       bounds.use ? bounds.min : 0.0f,
                                       bounds.use ? bounds.max : 0.0f,
                                       "%.3f");
        bool activated = ImGui::IsItemActivated();
        bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
        ImGui::EndDisabled();
        render_property_tracking_menu("track_property_context_value##" + unique_id, location, key_type);
        pop_tracked_value_style(is_tracked);

        if (!is_readonly) {
            if (activated || edited || deactivated) {
                log_info("FLOAT [{}]: activated={}, edited={}, deactivated={}, old={}, new={}", 
                         key, activated, edited, deactivated, 
                         value.IsDouble() ? value.GetDouble() : value.GetFloat(), float_value);
            }

            if (activated) {
                editing_original_values[unique_id] = float_value;
                log_info("Captured old value for {}: {}", key, float_value);
            }

            if (edited) {
                value.SetFloat(float_value);
            }

            if (deactivated) {
                log_info("Deactivated for {}", key);
                auto it = editing_original_values.find(unique_id);
                if (it != editing_original_values.end()) {
                    float old_value = std::get<float>(it->second);

                    log_info("Found old value: {}, new value: {}", old_value, float_value);

                    if (old_value != float_value) {
                        log_info("Creating command for {} change: {} -> {}", key, old_value, float_value);
                        auto command = std::make_unique<PropertyChangeCommand>(
                            PropertyLocation{entity_id, variant_type, current_path},
                            old_value,
                            float_value
                        );
                        CommandManager::get().execute_command(std::move(command));
                    } else {
                        log_info("Values are equal, no command needed");
                    }

                    editing_original_values.erase(it);
                } else {
                    log_error("Deactivated for {} but no original value found!", key);
                }
            }
        }
    } 
    else if (value.IsBool()) {
        bool bool_value = value.GetBool();
        bool old_value = bool_value;

        push_tracked_value_style(is_tracked);
        ImGui::BeginDisabled(is_readonly);
        bool edited = ImGui::Checkbox(("##" + unique_id).c_str(), &bool_value);
        ImGui::EndDisabled();
        render_property_tracking_menu("track_property_context_value##" + unique_id, location, key_type);
        pop_tracked_value_style(is_tracked);

        if (!is_readonly && edited) {
            log_info("BOOL [{}]: changed from {} to {}", key, old_value, bool_value);
            value.SetBool(bool_value);
            auto command = std::make_unique<PropertyChangeCommand>(
                PropertyLocation{entity_id, variant_type, current_path},
                old_value,
                bool_value
            );
            CommandManager::get().execute_command(std::move(command));
        }
    } 
    else if (value.IsString()) {
        char buffer[256];
        strncpy(buffer, value.GetString(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        push_tracked_value_style(is_tracked);
        ImGui::BeginDisabled(is_readonly);
        bool edited = ImGui::InputText(("##" + unique_id).c_str(), buffer, sizeof(buffer));
        bool activated = ImGui::IsItemActivated();
        bool deactivated = ImGui::IsItemDeactivatedAfterEdit();
        ImGui::EndDisabled();
        render_property_tracking_menu("track_property_context_value##" + unique_id, location, key_type);
        pop_tracked_value_style(is_tracked);

        if (!is_readonly) {
            if (activated || edited || deactivated) {
                log_info("STRING [{}]: activated={}, edited={}, deactivated={}, old='{}', new='{}'", 
                         key, activated, edited, deactivated, value.GetString(), buffer);
            }

            if (activated) {
                editing_original_values[unique_id] = std::string(value.GetString());
                log_info("Captured old value for {}: '{}'", key, value.GetString());
            }

            if (edited) {
                value.SetString(buffer, document.GetAllocator());
            }

            if (deactivated) {
                log_info("Deactivated for {}", key);
                auto it = editing_original_values.find(unique_id);
                if (it != editing_original_values.end()) {
                    std::string old_value = std::get<std::string>(it->second);
                    std::string new_str_value = buffer;

                    log_info("Found old value: '{}', new value: '{}'", old_value, new_str_value);

                    if (old_value != new_str_value) {
                        log_info("Creating command for {} change: '{}' -> '{}'", key, old_value, new_str_value);
                        auto command = std::make_unique<PropertyChangeCommand>(
                            PropertyLocation{entity_id, variant_type, current_path},
                            old_value,
                            new_str_value
                        );
                        CommandManager::get().execute_command(std::move(command));
                    } else {
                        log_info("Values are equal, no command needed");
                    }

                    editing_original_values.erase(it);
                } else {
                    log_error("Deactivated for {} but no original value found!", key);
                }
            }
        }
    }

    ImGui::PopItemWidth();
}

void Inspector::Impl::render_array_field(rapidjson::Document& document, rapidjson::Value& value,
                                    uint64_t entity_id, const std::string& variant_type,
                                    const std::string& key, const std::string& current_path) {
    std::string header_label = key + " [" + std::to_string(value.Size()) + "]";

    if (ImGui::TreeNode(header_label.c_str())) {
        for (rapidjson::SizeType i = 0; i < value.Size(); ++i) {
            ImGui::PushID(static_cast<int>(i));

            std::string item_path = current_path + "[" + std::to_string(i) + "]";
            std::string item_label = "[" + std::to_string(i) + "]";
            rapidjson::Value& item = value[i];

            if (item.IsObject()) {
                if (ImGui::TreeNode(item_label.c_str())) {
                    render_object(document, item, entity_id, variant_type, item_path);
                    ImGui::TreePop();
                }
            } else {
                render_property(document, item, entity_id, variant_type, item_label, item_path);
            }

            ImGui::PopID();
        }
        ImGui::TreePop();
    }
}

void Inspector::Impl::render_add_component_button(EntityDocument& entity, uint64_t entity_id) {
    if (ImGui::Button("+ Add Component")) {
        ImGui::OpenPopup("add_component_popup");
    }

    if (ImGui::BeginPopup("add_component_popup")) {
        for (auto& variant_doc : variants) {
            rapidjson::Document& variant_json = variant_doc.get_document();
            if (!variant_json.HasMember("type")) {
                continue;
            }

            const std::string variant_type = variant_json["type"].GetString();
        	const bool already_exists = entity.has_component(variant_type);

        	if (already_exists) {
            	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            	ImGui::MenuItem(variant_type.c_str(), nullptr, false, false);
            	ImGui::PopStyleColor();
        	} else if (ImGui::MenuItem(variant_type.c_str())) {
            	auto command = std::make_unique<AddComponentCommand>(entity_id, variant_type);
            	CommandManager::get().execute_command(std::move(command));
        	}
        }

        ImGui::EndPopup();
    }
}
