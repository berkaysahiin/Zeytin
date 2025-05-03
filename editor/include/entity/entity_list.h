#pragma once

#include "entity_document.h"

#include <vector>
#include <filesystem>

class EntityList final {
public:
    EntityList();

    ~EntityList() {
        clean_backup_entities();
    }

    inline std::vector<EntityDocument>& get_entities() { return m_entities; }
    std::string as_string() const;
    
private:

    void register_event_handlers();
    void sync_entities_from_document(const rapidjson::Document& document);

    void load_entity_from_file(const std::filesystem::path& path);
    void load_entities(const std::filesystem::path& path);
    void save_entities();

    void backup_entities();
    void clean_backup_entities();

    inline bool should_sync_runtime() { return !m_is_synced_once || m_is_play_mode; }

    bool m_is_play_mode = false;
    bool m_is_synced_once = false;

    std::vector<EntityDocument> m_entities;
};
