#include "resource_manager/resource_manager.h"
#include "logger.h"

namespace {
    const char* ENGINE = "engine";
    const char* EDITOR = "editor";
    const char* SHARED_RESOUCES = "shared_resources";
}

ResourceManager::ResourceManager() {
    construct_paths();
}

void ResourceManager::construct_paths() {
    std::filesystem::path current_dir = get_search_start_dir();

    const int MAX_LEVELS = 10;
    int level = 0;

    while(level < MAX_LEVELS) {
        bool found_engine = std::filesystem::exists(current_dir / ENGINE);
        bool found_editor = std::filesystem::exists(current_dir / EDITOR);

        if(found_engine && found_editor) { // these 2 should always be there
            m_root_path = current_dir;

            m_engine_path = current_dir/ENGINE;
            m_editor_path = current_dir/EDITOR;
            m_resources_path = current_dir/SHARED_RESOUCES;

            bool has_resouces = std::filesystem::exists(m_resources_path);

            if(!has_resouces) { // resources folder may not be there, create one
                std::filesystem::create_directory(m_resources_path);
            }

            log_info() << "[ResourceManager] Project root found at: " << m_root_path << std::endl;
            log_info() << "[ResourceManager] Engine path: " << m_engine_path << std::endl;
            log_info() << "[ResourceManager] Editor path: " << m_editor_path << std::endl;
            log_info() << "[ResourceManager] Resources path: " << m_resources_path << std::endl;

            return; // paths are constructred
        }

        // move up one directory, if not found
        current_dir = current_dir.parent_path();
        level++;
    }

    // if we get here, we couldnt find a valid project structure
    // may crash or abort, but we also want log_error to be visible, so just log and return
    log_error() << "ERROR: Could not locate project root structure!" << std::endl;
}

std::filesystem::path ResourceManager::get_search_start_dir() const {
    return std::filesystem::current_path();
}

std::filesystem::path ResourceManager::get_resource_subdir(const std::filesystem::path& subdir) const {
    std::filesystem::path full_path = m_resources_path / subdir;

    if(!std::filesystem::exists(full_path)) {
        log_warning() << "Folder: " << full_path << " does not exist. Creating full path" << std::endl;
        std::filesystem::create_directories(full_path);
    }
    return full_path;
}

std::filesystem::path ResourceManager::get_engine_subdir(const std::filesystem::path& subdir) const {
    std::filesystem::path full_path = m_engine_path / subdir;

    if(!std::filesystem::exists(full_path)) {
        log_error() << "Engine subdir: " << full_path << " does not exist." << std::endl;
    }
    return full_path;
}

std::filesystem::directory_iterator ResourceManager::get_variant_folder() const {
    const std::filesystem::path variants_path = get_variants_path(); // ensured to exist
    return std::filesystem::directory_iterator(variants_path);
}

std::filesystem::path ResourceManager::get_variant_path(const std::string& name) const {
    return get_variants_path() / (name + ".variant");
}
