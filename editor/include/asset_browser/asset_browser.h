#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
#include "raylib.h"

#include "utility/singleton.h"

namespace fs = std::filesystem;

enum class AssetType {
    Unknown,
    Image,
    Script,
    Entity,
    Variant,
    Scene,
    Other
};

struct AssetItem {
    std::string name;
    fs::path path;
    std::string extension;
    bool is_directory;
    AssetType type = AssetType::Unknown;
};

class AssetBrowser {
    MAKE_SINGLETON(AssetBrowser)

public:
    void render();
    void refresh();

    AssetType determine_asset_type(const std::string& extension);
    void render_directory_tree();
    void render_content_view();
    
    fs::path m_root_path;
    fs::path m_current_path;
    fs::path m_selected_path;
    
    struct FileInfo {
        std::string name;
        fs::path path;
        std::string extension;
        AssetType type;
    };

    struct DirectoryInfo {
        std::vector<FileInfo> files;
        std::vector<fs::path> subdirectories;
    };

    std::unordered_map<fs::path, DirectoryInfo> m_directory_cache;
    std::unordered_map<fs::path, Texture2D> m_image_cache;
    
    bool m_show_previews = true;
    void load_directory(const fs::path& path);
private:
    AssetBrowser();
    ~AssetBrowser();
};
