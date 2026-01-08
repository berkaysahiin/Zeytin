module;

#include "rapidjson/document.h"

#include <string>
#include <memory>
#include <fstream>
#include <filesystem>

module zeytin.variant.document;
import zeytin.logger;
import zeytin.resource;
import zeytin.validation;

struct VariantDocument::Impl {
    std::string name;
    rapidjson::Document document;
    bool is_dead_flag = false;
    
    Impl(std::string variant_name);
    
    // Structure enforcement
    void ensure_valid_structure();
    
    // Validation
    ValidationResult validate() const;
    
    // Serialization
    void load_from_file();
};

VariantDocument::Impl::Impl(std::string variant_name) 
    : name(std::move(variant_name)) {
}

void VariantDocument::Impl::ensure_valid_structure() {
    if (!document.IsObject()) {
		return; // Not yet
    }
    
    bool structure_was_invalid = false;
    
    // ensure type field exists
    if (!document.HasMember("type")) {
        log_warning("Variant '{}' missing 'type' field, adding it", name);
        rapidjson::Value type_value;
        type_value.SetString(name.c_str(), static_cast<rapidjson::SizeType>(name.length()), document.GetAllocator());
        document.AddMember("type", type_value, document.GetAllocator());
        structure_was_invalid = true;
    } else if (!document["type"].IsString()) {
        log_error("Variant '{}' has 'type' member but it's not a string, fixing it", name);
        document["type"].SetString(name.c_str(), static_cast<rapidjson::SizeType>(name.length()), document.GetAllocator());
        structure_was_invalid = true;
    }
    
    if (structure_was_invalid) {
        log_warning("Variant '{}' structure was corrected - this may indicate a parsing or serialization error", name);
    }
}

ValidationResult VariantDocument::Impl::validate() const {
    ValidationResult result = ValidationResult::success();
    
    if (!document.HasMember("type")) {
        result.add_error("Variant doesn't have a type field");
    } else if (!document["type"].IsString()) {
        result.add_error("Variant type is not a string");
    } else if (std::string(document["type"].GetString()) != name) {
        result.add_error("Variant type field doesn't match variant name");
    }
    
    return result;
}

void VariantDocument::Impl::load_from_file() {
    if (name.empty()) {
        log_error("Error: Cannot load variant with empty name");
        return;
    }

    const std::filesystem::path path = ResourceManager::get().get_component_path(name);

    std::ifstream in_file(path);
    if (!in_file.is_open()) {
        log_error("Error: Failed to open variant file: {}", path.string());
        return;
    }

    std::string json_string;
    try {
        json_string = std::string(
            std::istreambuf_iterator<char>(in_file),
            std::istreambuf_iterator<char>()
        );
    } catch (const std::exception& e) {
        log_error("Error: Failed to read variant file: {}. What: {}", path.string(), e.what());
        in_file.close();
        return;
    }

    in_file.close();

    if (json_string.empty()) {
        log_error("Error: Variant file is empty: {}", path.string());
        return;
    }

    document.Parse(json_string.c_str());

    if (document.HasParseError()) {
        log_error("Error: JSON parse error in variant file: {}", path.string());
        log_error("  Error offset: {}", document.GetErrorOffset());
        return;
    }

    if (!document.IsObject()) {
        log_error("Error: Variant file does not contain a valid JSON object: {}", path.string());
        return;
    }
    
    // Remove annotations section if present
    if (document.HasMember("annotations")) {
        document.RemoveMember("annotations");
    }
    
    // Ensure the loaded document has valid structure
    ensure_valid_structure();
}

VariantDocument::VariantDocument(std::string name) 
    : pImpl(std::make_unique<Impl>(std::move(name))) {
    pImpl->ensure_valid_structure();
}

VariantDocument::VariantDocument(rapidjson::Document document, std::string name) 
    : pImpl(std::make_unique<Impl>(std::move(name))) {
    pImpl->document = std::move(document);
    pImpl->ensure_valid_structure();
}

VariantDocument::~VariantDocument() = default;

VariantDocument::VariantDocument(VariantDocument&&) noexcept = default;

VariantDocument& VariantDocument::operator=(VariantDocument&&) noexcept = default;

const std::string& VariantDocument::get_name() const {
    return pImpl->name;
}

bool VariantDocument::is_dead() const {
    return pImpl->is_dead_flag;
}

void VariantDocument::mark_dead() {
    pImpl->is_dead_flag = true;
}

void VariantDocument::set_alive() {
    pImpl->is_dead_flag = false;
}

bool VariantDocument::is_valid() const {
    ValidationResult result = pImpl->validate();
    
    if (result.is_invalid()) {
        std::string variant_name = pImpl->name.empty() ? "_unknown_variant_" : pImpl->name;
        log_error("Variant validation failed ({}). Errors:\n{}", 
                  variant_name, 
                  result.get_all_errors());
    }
    
    return result.is_valid();
}

void VariantDocument::load_from_file() {
    pImpl->load_from_file();
}

rapidjson::Document& VariantDocument::get_document() {
    return pImpl->document;
}

const rapidjson::Document& VariantDocument::get_document() const {
    return pImpl->document;
}
