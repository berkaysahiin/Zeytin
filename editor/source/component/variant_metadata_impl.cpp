module;

#include <optional>
#include <fstream>
#include <unordered_map>
#include <filesystem>
#include <mutex>
#include <chrono>
#include "rapidjson/document.h"

module zeytin.variant.metadata;
import zeytin.filewatcher;
import zeytin.logger;

struct VariantMetadata::Impl {
    // variant_name -> property_name -> {annotation_key: annotation_value}
    std::unordered_map<std::string, 
        std::unordered_map<std::string, 
            std::unordered_map<std::string, std::string>>> metadata;

    std::filesystem::path components_dir;
    std::unique_ptr<FileWatcher> watcher;
    bool watcher_started = false;
    mutable std::mutex mutex;
};

VariantMetadata::VariantMetadata() 
    : pImpl(std::make_unique<Impl>()) {
}

VariantMetadata::~VariantMetadata() = default;

bool VariantMetadata::has_annotation(
    const std::string& variant_name, 
    const std::string& property_name, 
    const std::string& annotation_key) const 
{
    std::scoped_lock lock(pImpl->mutex);

    auto variant_it = pImpl->metadata.find(variant_name);
    if (variant_it == pImpl->metadata.end()) {
        return false;
    }
    
    auto property_it = variant_it->second.find(property_name);
    if (property_it == variant_it->second.end()) {
        return false;
    }
    
    return property_it->second.find(annotation_key) != property_it->second.end();
}

bool VariantMetadata::has_any_annotation(
    const std::string& variant_name, 
    const std::string& property_name) const 
{
    std::scoped_lock lock(pImpl->mutex);

    auto variant_it = pImpl->metadata.find(variant_name);
    if (variant_it == pImpl->metadata.end()) {
        return false;
    }
    
    auto property_it = variant_it->second.find(property_name);
    if (property_it == variant_it->second.end()) {
        return false;
    }
    
    return true;
}

std::optional<std::string> VariantMetadata::get_annotation(
    const std::string& variant_name,
    const std::string& property_name,
    const std::string& annotation_key) const 
{
    std::scoped_lock lock(pImpl->mutex);

    auto variant_it = pImpl->metadata.find(variant_name);
    if (variant_it == pImpl->metadata.end()) {
        return std::nullopt;
    }
    
    auto property_it = variant_it->second.find(property_name);
    if (property_it == variant_it->second.end()) {
        return std::nullopt;
    }
    
    auto annotation_it = property_it->second.find(annotation_key);
    if (annotation_it == property_it->second.end()) {
        return std::nullopt;
    }
    
    return annotation_it->second;
}
	
bool VariantMetadata::add_annotation(const std::string& variant_name,
                                              const std::string& property_name,
                                              std::string annotation_key, std::string annotation_value)
{
    std::scoped_lock lock(pImpl->mutex);
	pImpl->metadata[variant_name][property_name] = {{annotation_key, annotation_value}};
	return true;
}

std::unordered_map<std::string, std::string> VariantMetadata::get_all_annotations(
    const std::string& variant_name,
    const std::string& property_name) const 
{
    std::unordered_map<std::string, std::string> result;

    std::scoped_lock lock(pImpl->mutex);

    auto variant_it = pImpl->metadata.find(variant_name);
    if (variant_it == pImpl->metadata.end()) {
        return result;
    }
    
    auto property_it = variant_it->second.find(property_name);
    if (property_it == variant_it->second.end()) {
        return result;
    }
    
    return property_it->second;
}

void VariantMetadata::load_from_component_files(const std::filesystem::path& components_dir) {
    if (!std::filesystem::exists(components_dir)) {
        log_error("Components directory not found: {}", components_dir.string());
        return;
    }

    std::unordered_map<std::string, 
        std::unordered_map<std::string, 
            std::unordered_map<std::string, std::string>>> new_metadata;

    int loaded_count = 0;

    for (const auto& entry : std::filesystem::directory_iterator(components_dir)) {
        if (entry.path().extension() != ".component") {
            continue;
        }
        
        // read file
        std::ifstream file(entry.path());
        if (!file.is_open()) {
            log_warning("Failed to open component file: {}", entry.path().string());
            continue;
        }
        
        std::string json_str((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
        file.close();
        
        // parse JSON
        rapidjson::Document doc;
        doc.Parse(json_str.c_str());
        
        if (doc.HasParseError()) {
            log_warning("Failed to parse component file: {}", entry.path().string());
            continue;
        }
        
        if (!doc.HasMember("type") || !doc.HasMember("annotations")) {
            continue;  // old format or no annotations
        }
        
        std::string variant_name = doc["type"].GetString();
        const auto& annotations = doc["annotations"];
        
        // parse annotations for each property
        for (auto it = annotations.MemberBegin(); it != annotations.MemberEnd(); ++it) {
            std::string property_name = it->name.GetString();
            const auto& property_annotations = it->value;
            
            if (!property_annotations.IsObject()) {
                continue;
            }
            
            // store each annotation key-value pair
            for (auto ann_it = property_annotations.MemberBegin(); 
                 ann_it != property_annotations.MemberEnd(); ++ann_it) {
                
                std::string annotation_key = ann_it->name.GetString();
                
                std::string annotation_value;
                if (ann_it->value.IsBool()) {
                    annotation_value = ann_it->value.GetBool() ? "true" : "false";
                } else if (ann_it->value.IsString()) {
                    annotation_value = ann_it->value.GetString();
                } else if (ann_it->value.IsNumber()) {
                    annotation_value = std::to_string(ann_it->value.GetDouble());
                }
                
                new_metadata[variant_name][property_name][annotation_key] = annotation_value;
            }
        }
        
        loaded_count++;
    }

    {
        std::scoped_lock lock(pImpl->mutex);
        pImpl->metadata = std::move(new_metadata);
        pImpl->components_dir = components_dir;
    }

    if (!pImpl->watcher_started) {
        pImpl->watcher = std::make_unique<FileWatcher>(components_dir, std::chrono::milliseconds(500));
        pImpl->watcher->add_callback({ ".component"}, [this](const std::filesystem::path&, const FileEvent) {
            std::filesystem::path components_dir;
            {
                std::scoped_lock lock(pImpl->mutex);
                components_dir = pImpl->components_dir;
            }
            if (!components_dir.empty()) {
                load_from_component_files(components_dir);
            }
        });
        pImpl->watcher->start();
        pImpl->watcher_started = true;
    }

    log_info("Loaded metadata for {} components", loaded_count);
}
