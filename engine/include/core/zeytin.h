#pragma once

#include <vector>
#include <filesystem>
#include <unordered_map>

#include "raylib_wrapper.h"

#include "core/guid/guid.h"
#include "entity/entity.h"
#include "editor/editor_communication.h"
#include "core/macros.h"
#include "rapidjson/document.h"
#include "rttr/variant.h"

constexpr float VIRTUAL_WIDTH = 1920;
constexpr float VIRTUAL_HEIGHT = 1080;

using VariantList = std::vector<rttr::variant>;
using Storage = std::unordered_map<entity_id, VariantList>;

struct State {
    bool started : 1;            
    bool late_started : 1;      
    bool should_die : 1;       
    bool scene_ready : 1;  
    bool play_mode : 1;   
    bool pause_play_mode : 1; 
    bool synced_once : 1;       
    bool reserved : 1; 
};

class Zeytin {
    MAKE_SINGLETON(Zeytin);
public:
    void initialize();
    void shutdown();
    void run_frame();
    inline bool should_die() const { return m_state.should_die || window_should_close(); }

    inline entity_id new_entity() { return generate_unique_id(); }
    void remove_variant(entity_id id, const rttr::type& type);
    void remove_entity(entity_id id);
    void clean_dead_variants();

    VariantList& get_variants(const entity_id& entity);

    std::string serialize_entity(const entity_id id);
    std::string serialize_entity(const entity_id id, const std::filesystem::path& path);
    entity_id deserialize_entity(const std::string& entity);

    bool load_scene(const std::filesystem::path&);
    std::string serialize_scene();
    bool deserialize_scene(const std::string& scene); 
    inline void reload_scene_expr() {
        deserialize_scene(serialize_scene());
    }

    void post_init_variants();
    void update_variants();
    void play_start_variants();
    void play_late_start_variants();
    void play_update_variants();
    void play_late_update_variants();

    inline Camera2D& get_camera() { return m_camera; }
    inline const Storage& get_storage() const { return m_storage; }
    inline Storage& get_storage() { return m_storage; }

    inline bool is_play_mode() const { return m_state.play_mode; }
    inline bool is_paused_play_mode() const { return m_state.pause_play_mode; }
    inline bool is_scene_ready() const { return m_state.scene_ready; }
    inline bool is_started() const { return m_state.started; }
    inline bool is_late_started() const { return m_state.late_started; }
    inline bool is_synced_once() const { return m_state.synced_once; }

#ifdef EDITOR_MODE
    void generate_variants();
    void subscribe_editor_events();
    void initial_sync_editor();
    void sync_editor();

    void enter_play_mode(const bool is_paused);
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

#ifdef EDITOR_MODE
    std::unique_ptr<EditorCommunication> m_editor_communication;
#endif
};
