#pragma once

#include <string>
#include <filesystem>
#include <vector>

struct Level {
    std::string name;
    std::filesystem::path path;
    
    Level() = default;
    Level(const std::string& n, const std::filesystem::path& p) 
        : name(n), path(p) {}
    
    bool is_valid() const {
        return !name.empty() && std::filesystem::exists(path);
    }
};

class LevelManager {
public:
    static std::vector<Level> get_all_levels();
    static Level get_level(const std::string& name);
    static std::string load_level(const Level& level);
    static std::string load_level(const std::string& level_name);
    
    static std::string load_scene_file(const std::string& scene_name);
    
private:
    static std::filesystem::path get_levels_directory();
    static std::filesystem::path get_scenes_directory();  // NEW
};
