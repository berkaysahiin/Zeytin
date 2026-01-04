module;

#include <algorithm>
#include <cstring>
#include <string>

export module zeytin.validation.entity;
import zeytin.validation;
import zeytin.entity.list;
import zeytin.entity.document;

export namespace entity_validation {
    
    ValidationResult validate_entity_name(const std::string& name, EntityList& entity_list) {
        ValidationResult result = ValidationResult::success();
        
        if (name.empty()) {
            result.add_error("Entity name cannot be empty");
            return result;
        }
        
        const char* invalid_chars = "/*:?\"<>|\\";
        for (char c : name) {
            if (strchr(invalid_chars, c) != nullptr) {
                result.add_error("Entity name contains invalid character: '" + std::string(1, c) + "'");
                return result;
            }
        }
        
        auto& entities = entity_list.get_entities();
        for (auto& entity : entities) {
            if (!entity.is_dead() && entity.get_name() == name) {
                result.add_error("Entity with name '" + name + "' already exists");
                return result;
            }
        }
        
        return result;
    }
    
    std::string sanitize_entity_name(std::string name) {
        name.erase(std::remove_if(name.begin(), name.end(),
            [](char c) { 
                return c == '/' || c == '\\' || c == ':' || c == '*' || 
                       c == '?' || c == '"' || c == '<' || c == '>' || c == '|'; 
            }), name.end());
        return name;
    }
}
