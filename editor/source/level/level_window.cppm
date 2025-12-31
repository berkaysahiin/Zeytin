module;

#include <string>

export module zeytin.windows.level;
import zeytin.singleton;
import zeytin.level;
import zeytin.entity.list;

export class LevelWindow : public Singleton<LevelWindow> {
	friend class Singleton<LevelWindow>;
public:
    void render();
    void set_entity_list(EntityList* entity_list) { m_entity_list = entity_list; }

private:
    LevelWindow() = default;
    void export_level_as_scene(const Level& level);
    
    EntityList* m_entity_list = nullptr;
    char m_new_level_name[64] = "";
    
    // For export feedback
    bool m_show_export_success = false;
    bool m_show_export_error = false;
    std::string m_export_message;
};
