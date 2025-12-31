module;

#include <string>
#include <filesystem>

export module zeytin.level;

export struct Level {
    std::string name;
    std::filesystem::path path;
    
    Level() = default;
    Level(const std::string& n, const std::filesystem::path& p) 
        : name(n), path(p) {}
    
    bool is_valid() const {
        return !name.empty() && std::filesystem::exists(path);
    }
};
