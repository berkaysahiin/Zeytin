#pragma once

#include <string>
#include <filesystem>
#include "utility/singleton.h"

#define VARIANT_FOLDER  "variants"
#define ENGINE_SCRIPTS_FOLDER "scripts"

class ResourceManager final {
    MAKE_SINGLETON(ResourceManager);

public:
    inline std::filesystem::path get_resources_path() const { return m_resources_path;}
    inline std::filesystem::path get_engine_path() const { return m_engine_path; }
    inline std::filesystem::path get_editor_path() const { return m_editor_path; }
    inline std::filesystem::path get_root_path() const { return m_root_path; }

    inline std::filesystem::path get_variants_path() const { return get_resource_subdir(VARIANT_FOLDER); }
    inline std::filesystem::path get_engine_scripts_path() const { return get_engine_subdir(ENGINE_SCRIPTS_FOLDER);}

    std::filesystem::directory_iterator get_variant_folder() const;

    std::filesystem::path get_resource_subdir(const std::filesystem::path& subdir) const;
    std::filesystem::path get_engine_subdir(const std::filesystem::path& subdir) const;

    std::filesystem::path get_variant_path(const std::string& name) const;

private:
    ResourceManager();

    void construct_paths();
    std::filesystem::path get_search_start_dir() const;

    std::filesystem::path m_resources_path;
    std::filesystem::path m_root_path;
    std::filesystem::path m_editor_path;
    std::filesystem::path m_engine_path;
};
