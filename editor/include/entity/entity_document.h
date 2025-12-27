#pragma once

#include <string>
#include <filesystem>
#include <rapidjson/document.h>

class EntityDocument final {

public:
    inline EntityDocument(std::string name) : m_name(name) {}
    inline EntityDocument(rapidjson::Document document, std::string name) : m_document(std::move(document)), m_name(name) {}

    inline rapidjson::Document& get_document() { return m_document; }
    inline const rapidjson::Document& get_document() const { return m_document; }

    inline void set_document(rapidjson::Document new_doc) { m_document = std::move(new_doc); }

    inline const std::string& get_name() const { return m_name ; }

    inline void mark_as_dead() { m_dead = true; }
    inline bool is_dead() const { return m_dead; }

    void save_to_file(const std::filesystem::path& path) const;
    void load_from_file(const std::filesystem::path& path); 

    std::string as_string() const;

private:
    std::string m_name;
    rapidjson::Document m_document;
    bool m_dead = false;
};
