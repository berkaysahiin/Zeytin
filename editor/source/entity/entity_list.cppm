module;

#include "rapidjson/document.h"

#include <optional>
#include <vector>
#include <filesystem>
#include <functional>

export module zeytin.entity.list;
import zeytin.entity.document;
import zeytin.level;

export class EntityList final {
public:
    using LevelUnloadingCallback = std::function<void(const Level&)>;  // RENAMED

    EntityList();

    ~EntityList() {
        clean_backup_entities();
    }

    std::optional<std::reference_wrapper<EntityDocument>> find_entity_by_id(uint64_t entity_id);

    void load_level(const Level& level);
    std::vector<Level> get_available_levels() const;
    const Level& get_current_level() const { return m_current_level; }
    bool is_play_mode() const { return m_is_play_mode; }

    inline std::vector<EntityDocument>& get_entities() { return m_entities; }
    std::string as_string() const;
    
    void save_all_entities();

    void add_level_unloading_callback(LevelUnloadingCallback callback);

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
    std::vector<LevelUnloadingCallback> m_level_unloading_callbacks;  
};
