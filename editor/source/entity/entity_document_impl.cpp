module;

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include <string>
#include <filesystem>
#include <cstdint>
#include <memory>
#include <fstream>
#include <utility>

module zeytin.entity.document;
import zeytin.logger;
import zeytin.validation;
import zeytin.utility.typedefs;

struct EntityDocument::Impl {
    String name;
    Path file_path;
    rapidjson::Document document;
    bool is_dead_flag = false;
    EntityDocumentID document_id = 0;
    
    Impl(String entity_name);
    
    // Structure enforcement
    void ensure_valid_structure();
    
    // Validation
    ValidationResult validate() const;
    
    // Component operations
    bool has_component(ConstRef<String> component_type) const;
    List<String> get_component_types() const;
    size_t get_component_count() const;
    
    // Serialization helpers
    String serialize() const;
    void deserialize_from_file(PathView path);
    void save(PathView path) const;
};

EntityDocument::Impl::Impl(String entity_name) 
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

bool EntityDocument::Impl::has_component(ConstRef<String> component_type) const {
    const auto& variants = document["variants"].GetArray();
    for (const auto& variant : variants) {
        if (variant.HasMember("type") && 
            variant["type"].IsString() &&
            String(variant["type"].GetString()) == component_type) {
            return true;
        }
    }
    
    return false;
}

List<String> EntityDocument::Impl::get_component_types() const {
    List<String> types;
    
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

String EntityDocument::Impl::serialize() const {
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

    if (!document.Accept(writer)) {
        log_error("Failed to serialize entity document");
        return "";
    }
    
    return buffer.GetString();
}

void EntityDocument::Impl::deserialize_from_file(PathView path) {
    if (!std::filesystem::exists(path)) {
        log_error("File does not exist: {}", path.string());
        return;
    }
    
    std::ifstream in_file(path);
    if (!in_file.is_open()) {
        log_error("Failed to open file: {}", path.string());
        return;
    }
    
    String json_string(
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

void EntityDocument::Impl::save(PathView path) const {
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

EntityDocument::EntityDocument(String name) 
    : pImpl(std::make_unique<Impl>(std::move(name))) {
    pImpl->ensure_valid_structure();
}

EntityDocument::EntityDocument(rapidjson::Document document, String name) 
    : pImpl(std::make_unique<Impl>(std::move(name))) {
    pImpl->document = std::move(document);
    pImpl->ensure_valid_structure();
}

EntityDocument::~EntityDocument() = default;

EntityDocument::EntityDocument(EntityDocument&&) noexcept = default;

EntityDocument& EntityDocument::operator=(EntityDocument&&) noexcept = default;

ConstRef<String> EntityDocument::get_name() const {
    return pImpl->name;
}

PathView EntityDocument::get_file_path() const {
    return pImpl->file_path;
}

EntityID EntityDocument::get_id() const {
    if (!pImpl->document.HasMember("entity_id") || 
        !pImpl->document["entity_id"].IsUint64()) {
        log_error("Entity {} does not have valid ID", pImpl->name);
        return 0;
    }
    
    return pImpl->document["entity_id"].GetUint64();
}

std::uint64_t EntityDocument::get_document_id() const {
    return pImpl->document_id;
}

void EntityDocument::set_document_id(const std::uint64_t id) {
    pImpl->document_id = id;
}

void EntityDocument::set_file_path(Path path) {
    pImpl->file_path = std::move(path);
}

bool EntityDocument::is_valid() const {
    ValidationResult result = pImpl->validate();
    
    if (result.is_invalid()) {
        String entity_name = pImpl->name.empty() ? "_unknown_entity_" : pImpl->name;
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

bool EntityDocument::has_component(ConstRef<String> component_type) const {
    return pImpl->has_component(component_type);
}

List<String> EntityDocument::get_component_types() const {
    return pImpl->get_component_types();
}

size_t EntityDocument::get_component_count() const {
    return pImpl->get_component_count();
}

void EntityDocument::save_to_file(ConstRef<Path> path) const {
    pImpl->save(path);
}

void EntityDocument::load_from_file() {
    if (pImpl->file_path.empty()) {
        log_error("EntityDocument: file path not set for {}", pImpl->name);
        return;
    }

    pImpl->deserialize_from_file(pImpl->file_path);
}

void EntityDocument::load_from_file(ConstRef<Path> path) {
    pImpl->file_path = path;
    pImpl->deserialize_from_file(path);
}

String EntityDocument::as_string() const {
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
