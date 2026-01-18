module;

#include <string>
#include <memory>
#include <rapidjson/document.h>

export module zeytin.component.document;
import zeytin.utility.typedefs;

export {
    using ComponentDocumentID = uint64;
}

export class ComponentDocument final {
public:
    ComponentDocument(std::string name);
    ComponentDocument(rapidjson::Document document, std::string name);
    ~ComponentDocument();
    
    ComponentDocument(const ComponentDocument&) = delete;
    ComponentDocument& operator=(const ComponentDocument&) = delete;
    ComponentDocument(ComponentDocument&&) noexcept;
    ComponentDocument& operator=(ComponentDocument&&) noexcept;
    
    const std::string& get_name() const;
    const Path& get_file_path() const;
    ComponentDocumentID get_id() const;
    void set_id(ComponentDocumentID id);
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
