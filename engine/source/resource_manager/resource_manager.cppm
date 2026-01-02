module;

#include <string>
#include <filesystem>

export module zeytin.resource;
import zeytin.singleton;

export class ResourceManager : public Singleton<ResourceManager> {
	friend class Singleton<ResourceManager>;
public:
    std::filesystem::path get_resources_path() const;
    std::filesystem::path get_entities_path() const; 
    std::filesystem::path get_components_path() const; 

    std::filesystem::directory_iterator get_entity_folder() const;
    std::filesystem::directory_iterator get_variant_folder() const;

    std::filesystem::path get_resource_subdir(const std::filesystem::path& subdir) const;
    std::filesystem::path get_variant_path(const std::string& name) const;
    std::filesystem::path get_entity_path(const std::string& name) const;

private:
    ResourceManager();

    void construct_paths();
    std::filesystem::path get_search_start_dir() const;
    std::filesystem::path m_resources_path;
};

