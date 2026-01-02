module;

#include <vector>
#include <filesystem>

#include "raylib.h"
#include "rapidjson/document.h"
#include "rttr/variant.h"

export module zeytin.zeytin;
import zeytin.entity;
import zeytin.raylib;
import zeytin.editor.communication;
import zeytin.editor.event;
import zeytin.singleton;

constexpr float VIRTUAL_WIDTH = 1920;
constexpr float VIRTUAL_HEIGHT = 1080;

export using ComponentList = std::vector<rttr::variant>;
export using Storage = std::unordered_map<EntityID, ComponentList>;

export struct State {
    bool started : 1;            
    bool late_started : 1;      
    bool should_die : 1;       
    bool scene_ready : 1;  
    bool play_mode : 1;   
    bool pause_play_mode : 1; 
    bool synced_once : 1;       
    bool reload_next_frame : 1; 
	bool load_level_next_frame : 1;
};

export class Zeytin : public Singleton<Zeytin> {
	friend class Singleton<Zeytin>;
public:
    void initialize();
    void shutdown();
    void run_frame();
    inline bool should_die() const { return m_state.should_die || window_should_close(); }

    EntityID new_entity();
    void remove_variant(EntityID id, const rttr::type& type);
    void remove_entity(EntityID id);
    void clean_dead_variants();

    ComponentList& get_components(const EntityID& entity);

    std::string serialize_entity(const EntityID id);
    std::string serialize_entity(const EntityID id, const std::filesystem::path& path);
    EntityID deserialize_entity(const std::string& entity);

    bool load_scene(const std::filesystem::path&);
    std::string serialize_scene();
    bool deserialize_scene(const std::string& scene); 
	bool switch_to_level(const std::string& level_name);

    void update_components();
    void play_start_components();
    void play_late_start_components();
    void play_update_components();
    void play_late_update_components();

    inline Camera2D& get_camera() { return m_camera; }
    inline const Storage& get_storage() const { return m_storage; }
    inline Storage& get_storage() { return m_storage; }

    inline bool is_play_mode() const { return m_state.play_mode; }
    inline bool is_paused_play_mode() const { return m_state.pause_play_mode; }
    inline bool is_scene_ready() const { return m_state.scene_ready; }
    inline bool is_started() const { return m_state.started; }
    inline bool is_late_started() const { return m_state.late_started; }
    inline bool is_synced_once() const { return m_state.synced_once; }

    inline void reload_scene() { m_state.reload_next_frame = true; } // processed in main loop
	void request_level_load(const std::string& level_name);

#ifdef EDITOR_MODE
    void subscribe_editor_events();
    void initial_sync_editor();
    void sync_editor();

    void enter_play_mode(const bool is_paused = false);
    void exit_play_mode();
    void pause_play_mode();

    void handle_entity_property_changed(const rapidjson::Document& doc);
    void handle_entity_variant_added(const rapidjson::Document& msg);
    void handle_entity_variant_removed(const rapidjson::Document& msg);
    void handle_entity_removed(const rapidjson::Document& msg);
#endif

private:
    Zeytin();
    ~Zeytin();
    void initialize_camera();
    void initialize_rendering();

#ifdef EDITOR_MODE
    void initialize_editor_communication();
#else
    void initialize_standalone();
#endif

    void update_camera();
    void render();

private:
    Storage m_storage;
    State m_state = {0};
    
    RenderTexture2D m_render_texture;
    Camera2D m_camera;
	std::string m_pending_level_name;  
	std::string m_current_level_name;  

#ifdef EDITOR_MODE
    std::unique_ptr<EditorCommunication> m_editor_communication;
#endif
};
