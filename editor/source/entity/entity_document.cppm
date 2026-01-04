module;

#include <string>
#include <filesystem>
#include <cstdint>
#include <optional>
#include <memory>
#include <vector>
#include <rapidjson/document.h>

export module zeytin.entity.document;

export class EntityDocument final {
public:
    EntityDocument(std::string name);
    EntityDocument(rapidjson::Document document, std::string name);  
    ~EntityDocument();
    
    EntityDocument(const EntityDocument&) = delete;
    EntityDocument& operator=(const EntityDocument&) = delete;
    EntityDocument(EntityDocument&&) noexcept;
    EntityDocument& operator=(EntityDocument&&) noexcept;
    
    const std::string& get_name() const;
    uint64_t get_id() const;
    bool is_valid() const;
    bool is_dead() const;
    void mark_as_dead();
    
    bool has_component(const std::string& component_type) const;
    std::vector<std::string> get_component_types() const;
    size_t get_component_count() const;
    
    void save_to_file(const std::filesystem::path& path) const;
    void load_from_file(const std::filesystem::path& path);
    std::string as_string() const;
    
    rapidjson::Document& get_document();
    const rapidjson::Document& get_document() const;
    void set_document(rapidjson::Document new_doc);  
	rapidjson::Value& get_components_json();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
