module;

#include <string>
#include <memory>
#include <rapidjson/document.h>

export module zeytin.variant.document;

export class VariantDocument final {
public:
    VariantDocument(std::string name);
    VariantDocument(rapidjson::Document document, std::string name);
    ~VariantDocument();
    
    VariantDocument(const VariantDocument&) = delete;
    VariantDocument& operator=(const VariantDocument&) = delete;
    VariantDocument(VariantDocument&&) noexcept;
    VariantDocument& operator=(VariantDocument&&) noexcept;
    
    const std::string& get_name() const;
    bool is_dead() const;
    void mark_dead();
    void set_alive();
    
    bool is_valid() const;
    
    void load_from_file();
    
    rapidjson::Document& get_document();
    const rapidjson::Document& get_document() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
