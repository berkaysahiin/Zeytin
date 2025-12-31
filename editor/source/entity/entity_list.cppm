module;

#include <vector>
#include <filesystem>
#include "rapidjson/document.h"

export module zeytin.entity.list;
import zeytin.entity.document;
import zeytin.level;

export class EntityList final {
public:
    EntityList();

    ~EntityList() {
        clean_backup_entities();
    }

	void load_level(const Level& level);
    std::vector<Level> get_available_levels() const;
    const Level& get_current_level() const { return m_current_level; }

    inline std::vector<EntityDocument>& get_entities() { return m_entities; }
    std::string as_string() const;
    
    // For Hierarchy to save entities
    void save_all_entities();

private:

    void register_event_handlers();
    void sync_entities_from_document(const rapidjson::Document& document);

    void load_entity_from_file(const std::filesystem::path& path);
    void load_entities(const std::filesystem::path& path);

    void backup_entities();
    void clean_backup_entities();

    inline bool should_sync_runtime() { return !m_is_synced_once || m_is_play_mode; }

    bool m_is_play_mode = false;
    bool m_is_synced_once = false;
	Level m_current_level;

    std::vector<EntityDocument> m_entities;
};
