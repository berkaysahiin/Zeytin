module;

#include "imgui.h"
#include <string>
#include <vector>
#include <variant>
#include <algorithm>
#include <memory>
#include <optional>

module zeytin.component.view;
import zeytin.component;
import zeytin.component.document;
import zeytin.component.registry;
import zeytin.component.instance;
import zeytin.property;
import zeytin.annotation;

// Forward declarations of static helpers
namespace {
	std::vector<ComponentID> get_component_ids_sorted();
	std::string annotation_value_to_string(const AnnotationValue& value);
	std::string property_value_to_string(const PropertyValue& value);
	void render_document_section(const ComponentDocument& doc, ComponentID component_id);
	void render_component_section(const Component& component);
	void render_instances_section(ComponentID component_id);
	void render_instance_details(const ComponentInstance& instance, const Component& component);
}

struct ComponentViewWindow::Impl {
	std::optional<ComponentID> selected_component_id;
	
	void render_component_list();
	void render_component_details();
};

ComponentViewWindow::ComponentViewWindow() 
	: pImpl(std::make_unique<Impl>()) {
}

ComponentViewWindow::~ComponentViewWindow() = default;

void ComponentViewWindow::render() {
	ImGui::BeginChild("ComponentListPane", ImVec2(300, 0), true);
	pImpl->render_component_list();
	ImGui::EndChild();
	
	ImGui::SameLine();
	
	ImGui::BeginChild("DetailsPane", ImVec2(0, 0), true);
	pImpl->render_component_details();
	ImGui::EndChild();
}

void ComponentViewWindow::Impl::render_component_list() {
	ImGui::Text("Components");
	ImGui::Separator();

	const auto component_ids = get_component_ids_sorted();
	if (component_ids.empty()) {
		ImGui::TextDisabled("No components available");
		return;
	}
	
	for (const auto& comp_id : component_ids) {
		auto component = get_component_const(comp_id);
		if (!component) {
			continue;
		}

		const bool is_selected = (selected_component_id.has_value() && selected_component_id.value() == comp_id);
		if (ImGui::Selectable(component->get().type.c_str(), is_selected)) {
			selected_component_id = comp_id;
		}
	}
}

void ComponentViewWindow::Impl::render_component_details() {
	if (!selected_component_id.has_value()) {
		ImGui::TextDisabled("Select a component to view details");
		return;
	}

	auto component_opt = get_component_const(selected_component_id.value());
	if (!component_opt) {
		ImGui::TextDisabled("Selected component missing");
		return;
	}

	const auto& component = component_opt->get();
	
	ImGui::Text("Component: %s", component.type.c_str());
	ImGui::Separator();
	
	// Get associated document
	auto doc_opt = get_document_from_component_const(selected_component_id.value());
	
	// ComponentDocument Section
	if (ImGui::CollapsingHeader("ComponentDocument", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (doc_opt) {
			render_document_section(doc_opt->get(), selected_component_id.value());
		} else {
			ImGui::TextDisabled("No document associated");
		}
	}
	
	// Component Section
	if (ImGui::CollapsingHeader("Component", ImGuiTreeNodeFlags_DefaultOpen)) {
		render_component_section(component);
	}
	
	// Instances Section
	if (ImGui::CollapsingHeader("Component Instances")) {
		render_instances_section(selected_component_id.value());
	}
}

namespace {

std::vector<ComponentID> get_component_ids_sorted() {
	auto ids = get_component_ids();
	std::sort(ids.begin(), ids.end());
	return ids;
}

std::string annotation_value_to_string(const AnnotationValue& value) {
	return std::visit([](const auto& val) -> std::string {
		using T = std::decay_t<decltype(val)>;
		if constexpr (std::is_same_v<T, std::monostate>) {
			return "(null)";
		} else if constexpr (std::is_same_v<T, bool>) {
			return val ? "true" : "false";
		} else if constexpr (std::is_same_v<T, int64_t>) {
			return std::to_string(val);
		} else if constexpr (std::is_same_v<T, double>) {
			return std::to_string(val);
		} else if constexpr (std::is_same_v<T, std::string>) {
			return val;
		}
		return "";
	}, value);
}

std::string property_value_to_string(const PropertyValue& value) {
	return std::visit([](const auto& val) -> std::string {
		using T = std::decay_t<decltype(val)>;
		if constexpr (std::is_same_v<T, std::monostate>) {
			return "(null)";
		} else if constexpr (std::is_same_v<T, bool>) {
			return val ? "true" : "false";
		} else if constexpr (std::is_same_v<T, int64_t>) {
			return std::to_string(val);
		} else if constexpr (std::is_same_v<T, double>) {
			return std::to_string(val);
		} else if constexpr (std::is_same_v<T, std::string>) {
			return "\"" + val + "\"";
		}
		return "";
	}, value);
}

void render_document_section(const ComponentDocument& doc, ComponentID component_id) {
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 2));
	
	if (ImGui::BeginTable("doc_info", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
		
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Document ID");
		ImGui::TableNextColumn();
		auto doc_id_opt = get_document_id_for_component(component_id);
		if (doc_id_opt) {
			ImGui::Text("%lu", doc_id_opt.value());
		} else {
			ImGui::TextDisabled("(none)");
		}
		
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Name");
		ImGui::TableNextColumn();
		ImGui::Text("%s", doc.get_name().c_str());
		
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("File Path");
		ImGui::TableNextColumn();
		ImGui::TextWrapped("%s", doc.get_file_path().string().c_str());
		
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Status");
		ImGui::TableNextColumn();
		if (doc.is_dead()) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "DEAD");
		} else {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Alive");
		}
		
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Valid");
		ImGui::TableNextColumn();
		if (doc.is_valid()) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Yes");
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No");
		}
		
		ImGui::EndTable();
	}
	
	ImGui::PopStyleVar();
}

void render_component_section(const Component& component) {
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 2));
	
	if (ImGui::BeginTable("comp_info", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
		
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Component ID");
		ImGui::TableNextColumn();
		ImGui::Text("%lu", component.id);
		
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Type");
		ImGui::TableNextColumn();
		ImGui::Text("%s", component.type.c_str());
		
		ImGui::EndTable();
	}
	
	ImGui::PopStyleVar();
	
	ImGui::Spacing();
	ImGui::Text("Properties (%zu)", component.properties.size());
	ImGui::Separator();
	
	if (component.properties.empty()) {
		ImGui::TextDisabled("No properties found");
	} else {
		if (ImGui::BeginTable("component_properties", 4, 
			ImGuiTableFlags_Borders | 
			ImGuiTableFlags_RowBg | 
			ImGuiTableFlags_Resizable | 
			ImGuiTableFlags_ScrollY)) {
			
			ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
			ImGui::TableSetupColumn("Default Value", ImGuiTableColumnFlags_WidthFixed, 150.0f);
			ImGui::TableSetupColumn("Annotations", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();
			
			for (const auto& property : component.properties) {
				ImGui::TableNextRow();
				
				ImGui::TableNextColumn();
				ImGui::Text("%lu", property.id);
				
				ImGui::TableNextColumn();
				ImGui::Text("%s", property.name.c_str());
				
				ImGui::TableNextColumn();
				const std::string value_str = property_value_to_string(property.value);
				ImGui::TextWrapped("%s", value_str.c_str());
				
				ImGui::TableNextColumn();
				
				auto ann_map_it = component.annotations.find(property.id);
				if (ann_map_it == component.annotations.end() || ann_map_it->second.empty()) {
					ImGui::TextDisabled("(no annotations)");
				} else {
					std::string annotations_display;
					
					for (const auto& [ann_key, ann_value] : ann_map_it->second) {
						if (!annotations_display.empty()) {
							annotations_display += ", ";
						}
						
						const std::string key_str = std::string(annotation_key_to_string(ann_key));
						annotations_display += key_str;
						
						const std::string value_str = annotation_value_to_string(ann_value);
						if (value_str != "true") {
							annotations_display += "=" + value_str;
						}
					}
					
					ImGui::TextWrapped("%s", annotations_display.c_str());
				}
			}

			ImGui::EndTable();
		}
	}
}

void render_instances_section(ComponentID component_id) {
	const auto& instance_ids = get_instances_for(component_id);
	
	ImGui::Text("Total Instances: %zu", instance_ids.size());
	ImGui::Separator();
	
	if (instance_ids.empty()) {
		ImGui::TextDisabled("No instances created");
		return;
	}
	
	auto component_opt = get_component_const(component_id);
	if (!component_opt) {
		ImGui::TextDisabled("Component not found");
		return;
	}
	
	const auto& component = component_opt->get();
	
	for (size_t i = 0; i < instance_ids.size(); ++i) {
		auto instance_opt = get_instance_const(instance_ids[i]);
		if (!instance_opt) {
			continue;
		}
		
		const auto& instance = instance_opt->get();
		
		std::string header = "Instance #" + std::to_string(i) + " (ID: " + std::to_string(instance.id) + ")";
		if (ImGui::TreeNode(header.c_str())) {
			render_instance_details(instance, component);
			ImGui::TreePop();
		}
	}
}

void render_instance_details(const ComponentInstance& instance, const Component& component) {
	ImGui::Text("Instance ID: %lu", instance.id);
	ImGui::Text("Component ID: %lu", instance.component_id);
	ImGui::Text("Overrides: %zu", instance.overrides.size());
	ImGui::Separator();
	
	if (ImGui::BeginTable("instance_properties", 3, 
		ImGuiTableFlags_Borders | 
		ImGuiTableFlags_RowBg | 
		ImGuiTableFlags_Resizable)) {
		
		ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 150.0f);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 150.0f);
		ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();
		
		for (const auto& property : component.properties) {
			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::Text("%s", property.name.c_str());
			
			ImGui::TableNextColumn();
			bool has_override = instance.overrides.contains(property.id);
			if (has_override) {
				const auto& override_value = instance.overrides.at(property.id);
				std::string value_str = property_value_to_string(override_value);
				ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "%s", value_str.c_str());
			} else {
				std::string value_str = property_value_to_string(property.value);
				ImGui::Text("%s", value_str.c_str());
			}
			
			ImGui::TableNextColumn();
			if (has_override) {
				ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "OVERRIDDEN");
			} else {
				ImGui::TextDisabled("Default");
			}
		}
		
		ImGui::EndTable();
	}
}

}
