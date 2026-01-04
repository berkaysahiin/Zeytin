module;

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include <string>
#include <filesystem>
#include <cstdint>
#include <optional>
#include <memory>
#include <vector>
#include <fstream>
#include <utility>

module zeytin.entity.document;
import zeytin.logger;
import zeytin.validation;

struct EntityDocument::Impl {
    std::string name;
    rapidjson::Document document;
    bool is_dead_flag = false;
    
    Impl(std::string entity_name);
    
    // Structure enforcement
    void ensure_valid_structure();
    
    // Validation
    ValidationResult validate() const;
    
    // Component operations
    bool has_component(const std::string& component_type) const;
    std::vector<std::string> get_component_types() const;
    size_t get_component_count() const;
    
    // Serialization helpers
    std::string serialize() const;
    void deserialize_from_file(const std::filesystem::path& path);
    void save(const std::filesystem::path& path) const;
};

EntityDocument::Impl::Impl(std::string entity_name) 
    : name(std::move(entity_name)) {
}

void EntityDocument::Impl::ensure_valid_structure() {
    if (!document.IsObject()) {
		return; // Not yet
    }
    
    bool structure_was_invalid = false;
    
    if (!document.HasMember("variants")) {
        log_warning("Entity '{}' missing 'variants' array, creating empty array", name);
        rapidjson::Value variants_array(rapidjson::kArrayType);
        document.AddMember("variants", variants_array, document.GetAllocator());
        structure_was_invalid = true;
    } else if (!document["variants"].IsArray()) {
        log_error("Entity '{}' has 'variants' member but it's not an array, replacing with empty array", name);
        document["variants"].SetArray();
        structure_was_invalid = true;
    }
    
    if (structure_was_invalid) {
        log_warning("Entity '{}' structure was corrected - this may indicate a parsing or serialization error", name);
    }
}

ValidationResult EntityDocument::Impl::validate() const {
    ValidationResult result = ValidationResult::success();
    
    if (!document.HasMember("entity_id")) {
        result.add_error("Entity doesn't have an ID");
    } else if (!document["entity_id"].IsUint64()) {
        result.add_error("Entity ID is not uint64_t");
    }

    if (!document.HasMember("variants")) {
        result.add_error("Entity doesn't have variants member");
    } else if (!document["variants"].IsArray()) {
        result.add_error("Entity variants is not an array");
    }

    return result;
}

bool EntityDocument::Impl::has_component(const std::string& component_type) const {
    const auto& variants = document["variants"].GetArray();
    for (const auto& variant : variants) {
        if (variant.HasMember("type") && 
            variant["type"].IsString() &&
            std::string(variant["type"].GetString()) == component_type) {
            return true;
        }
    }
    
    return false;
}

std::vector<std::string> EntityDocument::Impl::get_component_types() const {
    std::vector<std::string> types;
    
    const auto& variants = document["variants"].GetArray();
    types.reserve(variants.Size());
    
    for (const auto& variant : variants) {
        if (variant.HasMember("type") && variant["type"].IsString()) {
            types.emplace_back(variant["type"].GetString());
        }
    }
    
    return types;
}

size_t EntityDocument::Impl::get_component_count() const {
    return document["variants"].GetArray().Size();
}

std::string EntityDocument::Impl::serialize() const {
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    if (!document.Accept(writer)) {
        log_error("Failed to serialize entity document");
        return "";
    }
    
    return buffer.GetString();
}

void EntityDocument::Impl::deserialize_from_file(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        log_error("File does not exist: {}", path.string());
        return;
    }
    
    std::ifstream in_file(path);
    if (!in_file.is_open()) {
        log_error("Failed to open file: {}", path.string());
        return;
    }
    
    std::string json_string(
        (std::istreambuf_iterator<char>(in_file)),
        std::istreambuf_iterator<char>()
    );
    in_file.close();
    
    document.Parse(json_string.c_str());
    
    if (document.HasParseError()) {
        log_error("JSON parse error at offset {}: {}", 
                  document.GetErrorOffset(), 
                  static_cast<int>(document.GetParseError()));
        return;
    }
    
    ensure_valid_structure();
}

void EntityDocument::Impl::save(const std::filesystem::path& path) const {
    if (is_dead_flag) {
        return;
    }

    std::filesystem::create_directories(path.parent_path());
    
    std::ofstream out_file(path);
    if (!out_file.is_open()) {
        log_error("Failed to open file for writing: {}", path.string());
        return;
    }
    
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    
    if (!document.Accept(writer)) {
        log_error("Failed to serialize JSON document");
        return;
    }
    
    out_file << buffer.GetString();
    
    if (out_file.fail()) {
        log_error("Failed to write to file: {}", path.string());
    }
    
    out_file.close();
}

EntityDocument::EntityDocument(std::string name) 
    : pImpl(std::make_unique<Impl>(std::move(name))) {
    pImpl->ensure_valid_structure();
}

EntityDocument::EntityDocument(rapidjson::Document document, std::string name) 
    : pImpl(std::make_unique<Impl>(std::move(name))) {
    pImpl->document = std::move(document);
    pImpl->ensure_valid_structure();
}

EntityDocument::~EntityDocument() = default;

EntityDocument::EntityDocument(EntityDocument&&) noexcept = default;

EntityDocument& EntityDocument::operator=(EntityDocument&&) noexcept = default;

const std::string& EntityDocument::get_name() const {
    return pImpl->name;
}

uint64_t EntityDocument::get_id() const {
    if (!pImpl->document.HasMember("entity_id") || 
        !pImpl->document["entity_id"].IsUint64()) {
        log_error("Entity {} does not have valid ID", pImpl->name);
        return 0;
    }
    
    return pImpl->document["entity_id"].GetUint64();
}

bool EntityDocument::is_valid() const {
    ValidationResult result = pImpl->validate();
    
    if (result.is_invalid()) {
        std::string entity_name = pImpl->name.empty() ? "_unknown_entity_" : pImpl->name;
        log_error("Entity validation failed ({}). Errors:\n{}", 
                  entity_name, 
                  result.get_all_errors());
    }
    
    return result.is_valid();
}

bool EntityDocument::is_dead() const {
    return pImpl->is_dead_flag;
}

void EntityDocument::mark_as_dead() {
    pImpl->is_dead_flag = true;
}

void EntityDocument::mark_as_alive() {
    pImpl->is_dead_flag = false;
}

bool EntityDocument::has_component(const std::string& component_type) const {
    return pImpl->has_component(component_type);
}

std::vector<std::string> EntityDocument::get_component_types() const {
    return pImpl->get_component_types();
}

size_t EntityDocument::get_component_count() const {
    return pImpl->get_component_count();
}

void EntityDocument::save_to_file(const std::filesystem::path& path) const {
    pImpl->save(path);
}

void EntityDocument::load_from_file(const std::filesystem::path& path) {
    pImpl->deserialize_from_file(path);
}

std::string EntityDocument::as_string() const {
    return pImpl->serialize();
}

rapidjson::Document& EntityDocument::get_document() {
    return pImpl->document;
}

const rapidjson::Document& EntityDocument::get_document() const {
    return pImpl->document;
}

void EntityDocument::set_document(rapidjson::Document new_doc) {
    pImpl->document = std::move(new_doc);
    pImpl->ensure_valid_structure();
}

rapidjson::Value& EntityDocument::get_components_json() {
	pImpl->ensure_valid_structure();
    return pImpl->document["variants"];
}
