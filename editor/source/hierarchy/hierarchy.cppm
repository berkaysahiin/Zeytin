module;

#include <vector>
#include <cstdint>
#include <string>

export module zeytin.hierarchy;
import zeytin.variant.document;
import zeytin.variant.list;
import zeytin.entity.document;
import zeytin.entity.list;

export class Hierarchy final {
public: 
    Hierarchy(std::vector<VariantDocument>& variants, EntityList* entity_list);
    void update();

private:
    void subscribe_events();
    void render_save_controls();
    void render_create_entity();
    void create_new_entity(const char* name);
    void render_entity(EntityDocument& entity);
    void handle_entity_context_menu(EntityDocument& entity_document, uint64_t entity_id);
    void render_add_variant_menu(EntityDocument& entity_document);
    void add_variant_to_entity(EntityDocument& entity_document, VariantDocument& variant_document);
    void add_required_variants_to_entity(EntityDocument& entity_document, const std::string& variant_type);
    bool check_variant_exists(const EntityDocument& entity_document, const std::string& variant_name);
    void save_all_entities();

    std::vector<VariantDocument>& m_variants;
    EntityList *m_entity_list = nullptr;
};
