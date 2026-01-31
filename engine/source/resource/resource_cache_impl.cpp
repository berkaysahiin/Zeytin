module;

#include "raylib.h"
#include <filesystem>
#include <string>

module zeytin.resource_cache;
import zeytin.resource;
import zeytin.resource.manager;

template<>
Texture2D* ResourceCache<Texture2D>::get_ptr(const std::string& path) {
    if (path.empty()) {
        return nullptr;
    }

    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
        return &it->second;
    }

    std::filesystem::path full_path = ResourceManager::get().get_asset_path(path);
    if (!std::filesystem::exists(full_path)) {
        return nullptr;
    }

    const std::string filename = full_path.filename().string();
    const bool is_symbol = filename.rfind("symbol_", 0) == 0;

    Texture2D texture{};
    if (is_symbol) {
        Image image = LoadImage(full_path.c_str());
        if (image.data == nullptr || image.width <= 0 || image.height <= 0) {
            return nullptr;
        }

        const int target_height = 512;
        if (image.height > target_height) {
            const float scale = static_cast<float>(target_height) / static_cast<float>(image.height);
            const int target_width = static_cast<int>(image.width * scale);
            ImageResize(&image, target_width, target_height);
        }

        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    } else {
        texture = LoadTexture(full_path.c_str());
    }

    if (texture.id == 0) {
        return nullptr;
    }

    auto result = m_cache.emplace(path, texture).first;
    m_last_modified[path] = std::filesystem::last_write_time(full_path);
    return &result->second;
}

template<>
void ResourceCache<Texture2D>::reload(const std::string& path) {
    auto it = m_cache.find(path);
    if (it == m_cache.end()) {
        return;
    }

    UnloadTexture(it->second);
    m_cache.erase(it);
    m_last_modified.erase(path);

    get_ptr(path);
}

template<>
void ResourceCache<Texture2D>::check_hot_reload() {
    for (auto& [path, texture] : m_cache) {
        std::filesystem::path full_path = ResourceManager::get().get_asset_path(path);
        if (!std::filesystem::exists(full_path)) {
            continue;
        }

        auto current_time = std::filesystem::last_write_time(full_path);
        auto last_time = m_last_modified[path];

        if (current_time != last_time) {
            UnloadTexture(texture);
            texture = LoadTexture(full_path.c_str());
            m_last_modified[path] = current_time;
        }
    }
}

template<>
void ResourceCache<Texture2D>::clear() {
    for (auto& [path, texture] : m_cache) {
        UnloadTexture(texture);
    }
    m_cache.clear();
    m_last_modified.clear();
}
