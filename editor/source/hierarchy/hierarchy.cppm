module;

#include <cstdint>

export module zeytin.hierarchy;
import zeytin.component.document;
import zeytin.entity.document;
import zeytin.entity.list;

export class Hierarchy final {
public: 
    Hierarchy(EntityList* entity_list);
    void update();

private:
    void subscribe_events();
    void render_save_controls();
    void render_create_entity();
    void create_new_entity(const char* name);
    void render_entity(EntityDocument& entity);
    void handle_entity_context_menu(EntityDocument& entity_document, uint64_t entity_id);
    void save_all_entities();

    EntityList* m_entity_list = nullptr;
};
