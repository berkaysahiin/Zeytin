module;

#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

module zeytin.level;
import zeytin.resource.manager;
import zeytin.logger;

std::filesystem::path LevelManager::get_levels_directory() {
    return ResourceManager::get().get_resource_subdir("levels");
}

std::filesystem::path LevelManager::get_scenes_directory() {
    return ResourceManager::get().get_resource_subdir("scenes");
}

std::vector<Level> LevelManager::get_all_levels() {
    std::vector<Level> levels;
    auto levels_path = get_levels_directory();
    
    if (!std::filesystem::exists(levels_path)) {
        //log_warning() << "Levels directory does not exist: " << levels_path << std::endl;
        return levels;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(levels_path)) {
        if (entry.is_directory()) {
            levels.emplace_back(entry.path().filename().string(), entry.path());
        }
    }
    
    return levels;
}

Level LevelManager::get_level(const std::string& name) {
    auto levels_path = get_levels_directory();
    auto level_path = levels_path / name;
    
    if (std::filesystem::exists(level_path) && std::filesystem::is_directory(level_path)) {
        return Level(name, level_path);
    }
    
    //log_error() << "Level not found: " << name << std::endl;
    return Level();
}

std::string LevelManager::load_scene_file(const std::string& scene_name) {
    const auto scenes_path = get_scenes_directory();
    
    std::string filename = scene_name;
    if (filename.find(".scene") == std::string::npos) {
        filename += ".scene";
    }
    
    const std::filesystem::path scene_file = scenes_path / filename;
    
    if (!std::filesystem::exists(scene_file)) {
        //log_warning() << "Scene file not found: " << scene_file << std::endl;
        return "";
    }
    
    std::ifstream file(scene_file);
    if (!file.is_open()) {
        //log_error() << "Failed to open scene file: " << scene_file << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    std::string content = buffer.str();
    
    if (content.empty()) {
        //log_error() << "Scene file is empty: " << scene_file << std::endl;
        return "";
    }
    
    //log_info() << "Loaded scene file: " << scene_file << std::endl;
    return content;
}

std::string LevelManager::load_level(const std::string& level_name) {
    // First try loading as .scene file from scenes directory
    std::string scene_content = load_scene_file(level_name);
    if (!scene_content.empty()) {
        return scene_content;
    }
    
    // Fall back to loading from levels directory (old system)
    Level level = get_level(level_name);
    return load_level(level);
}

std::string LevelManager::load_level(const Level& level) {
    if (!level.is_valid()) {
        //log_error() << "Cannot load invalid level" << std::endl;
        return "";
    }
    
    rapidjson::Document document;
    document.SetObject();
    auto& allocator = document.GetAllocator();
    
    document.AddMember("type", "scene", allocator);
    
    rapidjson::Value entities_array(rapidjson::kArrayType);
    
    for (const auto& entry : std::filesystem::directory_iterator(level.path)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".entity") {
            continue;
        }
        
        std::ifstream file(entry.path());
        if (!file.is_open()) {
            //log_error() << "Failed to open entity file: " << entry.path() << std::endl;
            continue;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string entity_json = buffer.str();
        
        if (entity_json.empty()) {
            //log_warning() << "Empty entity file: " << entry.path() << std::endl;
            continue;
        }
       
        rapidjson::Document entity_doc;
        rapidjson::ParseResult parse_result = entity_doc.Parse(entity_json.c_str());
        
        if (parse_result.IsError()) {
            //log_error() << "Failed to parse entity file: " << entry.path() 
            //           << " Error: " << parse_result.Code() << std::endl;
            continue;
        }
        
        rapidjson::Value entity_value;
        entity_value.CopyFrom(entity_doc, allocator);
        entities_array.PushBack(entity_value, allocator);
    }
    
    document.AddMember("entities", entities_array, allocator);
    
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    
    if (!document.Accept(writer)) {
        //log_error() << "Failed to serialize level: " << level.name << std::endl;
        return "";
    }
    
    //log_info() << "Loaded level: " << level.name 
    //           << " with " << entities_array.Size() << " entities" << std::endl;
    
    return std::string(buffer.GetString(), buffer.GetSize());
}
