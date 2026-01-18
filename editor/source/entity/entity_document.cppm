module;

#include <cstdint>
#include <memory>
#include <rapidjson/document.h>

export module zeytin.entity.document;
import zeytin.utility.typedefs;

export using EntityID = std::uint64_t;
export using EntityDocumentID = std::uint64_t;

export class EntityDocument final {
public:
    EntityDocument(String name);
    EntityDocument(rapidjson::Document document, String name);
    ~EntityDocument();
    
    EntityDocument(ConstRef<EntityDocument>) = delete;
    EntityDocument& operator=(ConstRef<EntityDocument>) = delete;
    EntityDocument(EntityDocument&&) noexcept;
    EntityDocument& operator=(EntityDocument&&) noexcept;
    
	[[nodiscard]]
    ConstRef<String> get_name() const;

	[[nodiscard]]
    PathView get_file_path() const;

	[[nodiscard]]
    EntityID get_id() const;

	[[nodiscard]]
    EntityDocumentID get_document_id() const;

	[[nodiscard]]
    bool is_valid() const;

	[[nodiscard]]
    bool is_dead() const;

	[[nodiscard]]
    bool has_component(ConstRef<String> component_type) const;

	[[nodiscard]]
    List<String> get_component_types() const;

	[[nodiscard]]
    size_t get_component_count() const;

	[[nodiscard]]
    String as_string() const;

	[[nodiscard]]
    rapidjson::Document& get_document();

	[[nodiscard]]
    ConstRef<rapidjson::Document> get_document() const;

	[[nodiscard]]
	Ref<rapidjson::Value> get_components_json();

    void mark_as_dead();
	void mark_as_alive();

    void set_document_id(EntityDocumentID id);
    void set_file_path(Path path);
    
    void save_to_file(ConstRef<Path> path) const;
    void load_from_file();
    void load_from_file(ConstRef<Path> path);
    
    void set_document(rapidjson::Document new_doc);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
