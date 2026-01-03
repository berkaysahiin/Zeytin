module;

#include <string>
#include <rapidjson/document.h>

export module zeytin.variant.document;

export class VariantDocument final {
public:
    inline VariantDocument(std::string name) : m_name(name) {}
    inline VariantDocument(rapidjson::Document document, std::string name) : m_document(std::move(document)), m_name(name) {}

    inline const rapidjson::Document& get_document() const { return m_document; }
    inline const std::string& get_name() const { return m_name ; }

    inline bool is_dead() const { return m_is_dead; }
    inline void mark_dead() { m_is_dead = true; }
    inline void set_alive() { m_is_dead = false; }

    void load_from_file();

private:
    bool m_is_dead = false;
    std::string m_name;
    rapidjson::Document m_document;
};
