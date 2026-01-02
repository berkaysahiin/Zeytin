module;

#include <filesystem>

module zeytin.resource;
import zeytin.logger;

#define ENTITY_FOLDER "entities"
#define VARIANT_FOLDER  "variants"
#define ENGINE_SCRIPTS_FOLDER "scripts"

namespace {
    const char* ENGINE = "engine";
    const char* EDITOR = "editor";
    const char* SHARED_RESOUCES = "shared_resources";
}

ResourceManager::ResourceManager() {
    construct_paths();
}

std::filesystem::path ResourceManager::get_resources_path() const {
	return m_resources_path;
}

std::filesystem::path ResourceManager::get_entities_path() const {
	return get_resource_subdir(ENTITY_FOLDER); 
}

std::filesystem::path ResourceManager::get_components_path() const {
	return get_resource_subdir(VARIANT_FOLDER); 
}

void ResourceManager::construct_paths() {
    std::filesystem::path current_dir = get_search_start_dir();

    const int MAX_LEVELS = 10;
    int level = 0;

    while(level < MAX_LEVELS) {
        bool found_resources = std::filesystem::exists(current_dir / SHARED_RESOUCES);

        if(found_resources) { 
            m_resources_path = current_dir/SHARED_RESOUCES;

            //log_info() << "[ResourceManager] Resources path: " << m_resources_path << std::endl;

            return; // resources are found
        }

        // move up one directory, if not found
        current_dir = current_dir.parent_path();
        level++;
    }

    // if we get here, we couldnt find a valid resources folder
    // if EDITOR_MODE, resources folder should be provided by editor, if standalone, nothing we can do
    //log_error() << "ERROR: Could not locate resources!" << std::endl;
}

std::filesystem::path ResourceManager::get_search_start_dir() const {
    return std::filesystem::current_path();
}

std::filesystem::path ResourceManager::get_resource_subdir(const std::filesystem::path& subdir) const {
    std::filesystem::path full_path = m_resources_path / subdir;

    if(!std::filesystem::exists(full_path)) {
        //log_warning() << "Folder: " << full_path << " does not exist. Creating full path" << std::endl;
        std::filesystem::create_directories(full_path);
    }
    return full_path;
}

std::filesystem::directory_iterator ResourceManager::get_entity_folder() const {
    const std::filesystem::path entities_path = get_entities_path(); // ensured to exist
    return std::filesystem::directory_iterator(entities_path);
}

std::filesystem::directory_iterator ResourceManager::get_variant_folder() const {
    const std::filesystem::path variants_path = get_entities_path(); // ensured to exist
    return std::filesystem::directory_iterator(variants_path);
}

std::filesystem::path ResourceManager::get_entity_path(const std::string& name) const {
    return get_entities_path() / (name + ".entity");
}

std::filesystem::path ResourceManager::get_variant_path(const std::string& name) const {
    return get_components_path() / (name + ".variant");
}
