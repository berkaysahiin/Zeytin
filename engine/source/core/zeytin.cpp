#include "core/zeytin.h"

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"

#include "core/json/from_json.h"
#include "core/json/to_json.h"
#include "core/guid/guid.h"
#include "core/raylib_wrapper.h"
#include "core/utils.h"
#include "core/profiling.h"

#include "raylib.h"
#include "variant/variant_base.h"
#include "resource_manager/resource_manager.h"
#include "config_manager/config_manager.h"

#ifdef EDITOR_MODE
#include "editor/editor_event.h"
#endif

Zeytin::Zeytin() {
    initialize();
}

Zeytin::~Zeytin() {
    shutdown();
}

void Zeytin::initialize() {

#ifdef EDITOR_MODE
    initialize_editor_communication();
    generate_variants();
    initial_sync_editor();
#else
    initialize_standalone();
#endif

    initialize_rendering();
    initialize_camera();
}

void Zeytin::initialize_rendering() {
    m_render_texture = load_render_texture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
}

void Zeytin::initialize_camera() {
    m_camera.offset = {0, 0};
    m_camera.target = {0, 0};
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;
}

#ifdef EDITOR_MODE
void Zeytin::initialize_editor_communication() {
    m_editor_communication = std::make_unique<EditorCommunication>();
    subscribe_editor_events();
    
    // wait for editor connection to be established
    while (!m_editor_communication->is_connection_confirmed() || !m_state.scene_ready) {
        m_editor_communication->raise_events();
        
        // black screen while waiting for connection
        begin_drawing();
        clear_background(BLACK);
        end_drawing();
    }
}
#else

void Zeytin::initialize_standalone() {
    std::string startup_scene = CONFIG_GET("startup_scene", std::string, "main.scene");
    std::filesystem::path scene_path = ResourceManager::get().get_resource_subdir("scenes") / startup_scene;
    
    if (!load_scene(scene_path)) {
        log_error() << "Failed to load startup scene: " << scene_path << std::endl;
        m_state.should_die = true;
        return;
    }
    
    m_state.play_mode = true;
}

#endif 

void Zeytin::shutdown() {

#ifdef EDITOR_MODE
    if (m_state.play_mode) {
        exit_play_mode();  
    }

    if (m_editor_communication) {
        m_editor_communication.reset();
    }
#endif

    if (m_render_texture.id != 0) {
        unload_render_texture(m_render_texture);
        m_render_texture.id = 0;
    }
    
    m_storage.clear();
}

void Zeytin::run_frame() {
    ZPROFILE_FUNCTION();
    
#ifdef EDITOR_MODE
    if (m_editor_communication) {
        m_editor_communication->raise_events();
    }
#endif

    begin_texture_mode(m_render_texture);
    clear_background(RAYWHITE);

    begin_mode2d(m_camera);
    
    post_init_variants();
    update_variants();
    
    end_mode2d();

    if (m_state.play_mode && !m_state.pause_play_mode) {
        play_start_variants();
        play_late_start_variants();
        play_update_variants();
    }

    end_texture_mode();

    begin_drawing();
    clear_background(BLACK);
    render();
    end_drawing();
}

VariantList& Zeytin::get_variants(const entity_id& entity) {
    return m_storage[entity];
}

void Zeytin::remove_entity(entity_id id) {
    auto it = m_storage.find(id);
    if (it != m_storage.end()) {
        m_storage.erase(it);
    }
}

void Zeytin::clean_dead_variants() {
    ZPROFILE_FUNCTION();
    
    for (auto& [entity_id, variants] : m_storage) {
        variants.erase(
            std::remove_if(variants.begin(), variants.end(),
                [](rttr::variant& variant) {
                    VariantBase& var_base = variant.get_value<VariantBase&>();
                    return var_base.is_dead;
                }
            ),
            variants.end()
        );
    }
}

std::string Zeytin::serialize_entity(const entity_id id) {
    return rttr_json::serialize_entity(id, get_variants(id));
}

std::string Zeytin::serialize_entity(const entity_id id, const std::filesystem::path& path) {
    return rttr_json::serialize_entity(id, get_variants(id), path);
}

entity_id Zeytin::deserialize_entity(const std::string& entity_json) {
    if (entity_json.empty()) {
        log_error() << "Cannot deserialize empty entity JSON" << std::endl;
        return 0;
    }
    
    entity_id id;
    std::vector<rttr::variant> variants;

    if (!rttr_json::deserialize_entity(entity_json, id, variants)) {
        log_error() << "Failed to deserialize entity" << std::endl;
        return 0;
    }

    auto& entity_variants = get_variants(id);
    entity_variants.clear();

    for (auto& var : variants) {
        VariantBase& base = var.get_value<VariantBase&>();
        base.on_init();
        entity_variants.push_back(std::move(var));
    }
    
    return id;
}

void Zeytin::remove_variant(entity_id id, const rttr::type& type) {
    auto& variants = get_variants(id);
    for (auto& variant : variants) {
        if (variant.get_type() == type) {
            VariantBase& base = variant.get_value<VariantBase&>();
            base.is_dead = true;
        }
    }
}

std::string Zeytin::serialize_scene() {
    ZPROFILE_FUNCTION();
    
    rapidjson::Document document;
    document.SetObject();

    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    rapidjson::Value entities_array(rapidjson::kArrayType);

    for (const auto& [entity_id, variants] : m_storage) {
        std::string entity_json = serialize_entity(entity_id);
        if (entity_json.empty()) {
            continue;
        }

        rapidjson::Document entity_doc;
        rapidjson::ParseResult parse_result = entity_doc.Parse(entity_json.c_str());
        
        if (parse_result.IsError()) {
            log_error() << "Failed to parse entity JSON for entity: " << entity_id << std::endl;
            continue;
        }

        rapidjson::Value entity_value;
        entity_value.CopyFrom(entity_doc, allocator);
        entities_array.PushBack(entity_value, allocator);
    }

    document.AddMember("type", "scene", allocator);
    document.AddMember("entities", entities_array, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return std::string(buffer.GetString(), buffer.GetSize());
}

bool Zeytin::load_scene(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        log_error() << "Scene file does not exist: " << path << std::endl;
        return false;
    }

    std::ifstream scene_file(path);
    if (!scene_file.is_open()) {
        log_error() << "Failed to open scene file: " << path << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << scene_file.rdbuf();
    scene_file.close();
    
    std::string scene_content = buffer.str();
    if (scene_content.empty()) {
        log_error() << "Scene file is empty: " << path << std::endl;
        return false;
    }
    
    return deserialize_scene(scene_content);
}

bool Zeytin::deserialize_scene(const std::string& scene) {
    ZPROFILE_FUNCTION();
    
    if (scene.empty()) {
        log_error() << "Cannot deserialize empty scene" << std::endl;
        return false;
    }
    
    m_storage.clear();
    
    m_state.started = false;
    m_state.late_started = false;

    rapidjson::Document scene_data;
    rapidjson::ParseResult parse_result = scene_data.Parse(scene.c_str());

    if (parse_result.IsError()) {
        log_error() << "Error parsing scene at offset " << parse_result.Offset() << std::endl;
        return false;
    }

    if (!scene_data.IsObject() || 
        !scene_data.HasMember("type") ||
        !scene_data["type"].IsString() || 
        strcmp(scene_data["type"].GetString(), "scene") != 0 ||
        !scene_data.HasMember("entities") || 
        !scene_data["entities"].IsArray()) {
        
        log_error() << "Invalid scene format" << std::endl;
        return false;
    }

    const rapidjson::Value& entities = scene_data["entities"];
    int successful_entities = 0;
    
    for (rapidjson::SizeType i = 0; i < entities.Size(); i++) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        entities[i].Accept(writer);
        
        std::string entity_str = buffer.GetString();
        entity_id entity_id = deserialize_entity(entity_str);
        
        if (entity_id != 0) {
            successful_entities++;
        }
    }

    if (successful_entities == 0 && entities.Size() > 0) {
        log_error() << "Failed to deserialize any entities from scene" << std::endl;
        return false;
    }

    log_info() << "Scene loaded with " << successful_entities << " entities" << std::endl;
    return true;
}

void Zeytin::post_init_variants() {
    ZPROFILE_ZONE_NAMED("Zeytin::post_init_variants()");

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            VariantBase& base = variant.get_value<VariantBase&>();
            
            if (base.is_dead || base.post_inited) {
                continue;
            }
            
            base.post_inited = true;
            
            {
                ZPROFILE_ZONE_NAMED("VariantBase::on_post_init()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),
                              base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                
                base.on_post_init();
            }
        }
    }
}

void Zeytin::update_variants() {
    ZPROFILE_ZONE_NAMED("Zeytin::update_variants()");

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            VariantBase& base = variant.get_value<VariantBase&>();
            
            if (base.is_dead) {
                continue;
            }
            
            {
                ZPROFILE_ZONE_NAMED("VariantBase::on_update()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),
                              base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                
                base.on_update();
            }
        }
    }
}

void Zeytin::play_update_variants() {
    ZPROFILE_ZONE_NAMED("Zeytin::play_update_variants()");

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            VariantBase& base = variant.get_value<VariantBase&>();
            
            if (base.is_dead) {
                continue;
            }
            
            {
                ZPROFILE_ZONE_NAMED("VariantBase::on_play_update()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),
                              base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                
                base.on_play_update();
            }
        }
    }
}

void Zeytin::play_start_variants() {
    ZPROFILE_ZONE_NAMED("Zeytin::play_start_variants()");

    if (m_state.started) {
        return;
    }
    
    m_state.started = true;

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            VariantBase& base = variant.get_value<VariantBase&>();
            
            if (base.is_dead) {
                continue;
            }
            
            {
                ZPROFILE_ZONE_NAMED("VariantBase::on_play_start()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),
                              base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                
                base.on_play_start();
            }
        }
    }
}

void Zeytin::play_late_start_variants() {
    ZPROFILE_ZONE_NAMED("Zeytin::play_late_start_variants()");

    if (m_state.late_started) {
        return;
    }
    
    m_state.late_started = true;

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            VariantBase& base = variant.get_value<VariantBase&>();
            
            if (base.is_dead) {
                continue;
            }
            
            {
                ZPROFILE_ZONE_NAMED("VariantBase::on_play_late_start()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),
                              base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                
                base.on_play_late_start();
            }
        }
    }
}

void Zeytin::render() {
    ZPROFILE_FUNCTION();
    
    float screen_width = get_screen_width();
    float screen_height = get_screen_height();
    
    float scale_factor = (screen_width / VIRTUAL_WIDTH) < (screen_height / VIRTUAL_HEIGHT) ? 
                         (screen_width / VIRTUAL_WIDTH) : (screen_height / VIRTUAL_HEIGHT);
    
    float render_width = VIRTUAL_WIDTH * scale_factor;
    float render_height = VIRTUAL_HEIGHT * scale_factor;
    
    float pos_x = (screen_width - render_width) * 0.5f;
    float pos_y = (screen_height - render_height) * 0.5f;
    
    draw_texture_pro(
        m_render_texture.texture,
        {0, 0, (float)m_render_texture.texture.width, (float)-m_render_texture.texture.height},
        {pos_x, pos_y, render_width, render_height},
        {0, 0},
        0.0f,
        WHITE
    );
}

#ifdef EDITOR_MODE

void Zeytin::subscribe_editor_events() {
    EditorEventBus::get().subscribe<const std::string&>(
        EditorEvent::Scene, 
        [this](const auto& scene) {
            if (deserialize_scene(scene)) {
                m_state.scene_ready = true;
            }
        }
    );

    EditorEventBus::get().subscribe<const rapidjson::Document&>(
        EditorEvent::EntityPropertyChanged, 
        [this](const rapidjson::Document& doc) {
            handle_entity_property_changed(doc);
        }
    );

    EditorEventBus::get().subscribe<const rapidjson::Document&>(
        EditorEvent::EntityVariantAdded, 
        [this](const rapidjson::Document& msg) {
            handle_entity_variant_added(msg);
        }
    );

    EditorEventBus::get().subscribe<const rapidjson::Document&>(
        EditorEvent::EntityVariantRemoved, 
        [this](const rapidjson::Document& msg) {
            handle_entity_variant_removed(msg);
        }
    );

    EditorEventBus::get().subscribe<const rapidjson::Document&>(
        EditorEvent::EntityRemoved, 
        [this](const rapidjson::Document& msg) {
            handle_entity_removed(msg);
        }
    );

    EditorEventBus::get().subscribe<bool>(
        EditorEvent::EnterPlayMode, 
        [this](bool is_paused) {
            clean_dead_variants();
            enter_play_mode(is_paused);
        }
    );

    EditorEventBus::get().subscribe<bool>(
        EditorEvent::ExitPlayMode, 
        [this](bool) {
            exit_play_mode();
        }
    );

    EditorEventBus::get().subscribe<bool>(
        EditorEvent::PausePlayMode, 
        [this](bool) {
            m_state.pause_play_mode = true;
        }
    );

    EditorEventBus::get().subscribe<bool>(
        EditorEvent::UnPausePlayMode, 
        [this](bool) {
            m_state.pause_play_mode = false;
        }
    );

    EditorEventBus::get().subscribe<bool>(
        EditorEvent::Die, 
        [this](bool) {
            m_state.should_die = true;
        }
    );
}

void Zeytin::handle_entity_property_changed(const rapidjson::Document& doc) {
    if (doc.HasParseError() || 
        !doc.HasMember("entity_id") || 
        !doc.HasMember("variant_type") || 
        !doc.HasMember("key_type") || 
        !doc.HasMember("key_path") || 
        !doc.HasMember("value")) {
        
        log_error() << "Invalid property change document format" << std::endl;
        return;
    }

    uint64_t entity_id = doc["entity_id"].GetUint64();
    const std::string& variant_type = doc["variant_type"].GetString();
    const std::string& key_type = doc["key_type"].GetString();
    const std::string& key_path = doc["key_path"].GetString();
    const std::string& value_str = doc["value"].GetString();

    auto& variants = get_variants(entity_id);
    for (auto& variant : variants) {
        if (variant.get_type().get_name() == variant_type) {

            std::vector<std::string> path_parts = split_path(key_path);
            if (path_parts.empty()) {
                log_error() << "Invalid key path: " << key_path << std::endl;
                return;
            }

            if (key_type == "int") {
                update_property(variant, path_parts, 0, std::stoi(value_str));
            }
            else if (key_type == "float") {
                update_property(variant, path_parts, 0, std::stof(value_str));
            }
            else if (key_type == "bool") {
                update_property(variant, path_parts, 0, (value_str == "true" || value_str == "1"));
            }
            else if (key_type == "string") {
                update_property(variant, path_parts, 0, value_str);
            }
            else {
                log_error() << "Unsupported property type: " << key_type << std::endl;
            }
            
            return;
        }
    }
    
    log_error() << "Variant " << variant_type << " not found on entity " << entity_id << std::endl;
}

void Zeytin::handle_entity_variant_added(const rapidjson::Document& msg) {
    if (msg.HasParseError() || 
        !msg.HasMember("entity_id") || 
        !msg.HasMember("variant_type")) {
        
        log_error() << "Invalid variant add document format" << std::endl;
        return;
    }

    entity_id entity_id = msg["entity_id"].GetUint64();
    const char* variant_type_name = msg["variant_type"].GetString();

    VariantCreateInfo info;
    info.entity_id = entity_id;
    
    std::vector<rttr::argument> args;
    args.push_back(info);

    rttr::type rttr_type = rttr::type::get_by_name(variant_type_name);
    if (!rttr_type.is_valid()) {
        log_error() << "Invalid variant type: " << variant_type_name << std::endl;
        return;
    }

    rttr::variant obj = rttr_type.create(args);
    if (!obj.is_valid()) {
        log_error() << "Failed to create variant of type: " << variant_type_name << std::endl;
        return;
    }

    auto& variants = get_variants(entity_id);
    
    for (const auto& existing : variants) {
        if (existing.get_type() == rttr_type) {
            log_warning() << "Entity " << entity_id << " already has a variant of type " 
                        << variant_type_name << std::endl;
            return;
        }
    }
    
    VariantBase& base = obj.get_value<VariantBase&>();
    base.on_init();
    variants.push_back(std::move(obj));
    
    log_info() << "Added variant " << variant_type_name << " to entity " << entity_id << std::endl;
}

void Zeytin::handle_entity_variant_removed(const rapidjson::Document& msg) {
    if (msg.HasParseError() || 
        !msg.HasMember("entity_id") || 
        !msg.HasMember("variant_type")) {
        
        log_error() << "Invalid variant remove document format" << std::endl;
        return;
    }

    entity_id entity_id = msg["entity_id"].GetUint64();
    const char* variant_type_name = msg["variant_type"].GetString();

    rttr::type rttr_type = rttr::type::get_by_name(variant_type_name);
    if (!rttr_type.is_valid()) {
        log_error() << "Invalid variant type: " << variant_type_name << std::endl;
        return;
    }

    remove_variant(entity_id, rttr_type);
    log_info() << "Removed variant " << variant_type_name << " from entity " << entity_id << std::endl;
}

void Zeytin::handle_entity_removed(const rapidjson::Document& msg) {
    if (msg.HasParseError() || !msg.HasMember("entity_id")) {
        log_error() << "Invalid entity remove document format" << std::endl;
        return;
    }

    entity_id entity_id = msg["entity_id"].GetUint64();

    remove_entity(entity_id);
    log_info() << "Removed entity " << entity_id << std::endl;
}

void Zeytin::enter_play_mode(bool is_paused) {
    if (m_state.play_mode) {
        return;
    }

    std::string scene = serialize_scene();
    if (scene.empty()) {
        log_error() << "Failed to serialize scene for play mode" << std::endl;
        return;
    }

    std::filesystem::create_directory("temp");
    std::ofstream scene_file("temp/backup.scene");
    if (!scene_file.is_open()) {
        log_error() << "Failed to create backup scene file" << std::endl;
        return;
    }
    
    scene_file << scene;
    scene_file.close();

    m_state.pause_play_mode = is_paused;
    m_state.play_mode = true;
    
    log_info() << "Entered play mode" << (is_paused ? " (paused)" : "") << std::endl;
}

void Zeytin::exit_play_mode() {
    m_state.started = false;
    m_state.late_started = false;
    m_state.play_mode = false;
    m_state.pause_play_mode = false;

    std::filesystem::path backup_path = "temp/backup.scene";
    if (!std::filesystem::exists(backup_path)) {
        log_error() << "Cannot exit play mode: scene backup not found" << std::endl;
        return;
    }

    m_storage.clear();

    if (!load_scene(backup_path)) {
        log_error() << "Failed to load scene from backup" << std::endl;
        return;
    }

    std::filesystem::remove_all("temp");
    
    log_info() << "Exited play mode" << std::endl;
}

void Zeytin::initial_sync_editor() {
    std::string scene = serialize_scene();
    if (!scene.empty()) {
        EditorEventBus::get().publish<std::string>(EditorEvent::SyncEditor, scene);
        log_info() << "Initial scene sync with editor" << std::endl;
    } else {
        log_error() << "Failed to serialize scene for initial sync" << std::endl;
    }
}

void Zeytin::sync_editor() {
    static float sync_timer = 0.0f;
    static const float SYNC_INTERVAL = 0.1f;
    
    sync_timer += get_frame_time();
    
    if (sync_timer >= SYNC_INTERVAL) {
        sync_timer = 0.0f;
        
        std::string scene = serialize_scene();
        if (!scene.empty()) {
            EditorEventBus::get().publish<std::string>(EditorEvent::SyncEditor, scene);
        }
    }
}

void Zeytin::generate_variants() {
    ZPROFILE_FUNCTION();
    
    auto variant_folder = ResourceManager::get().get_variant_folder();
    for (const auto& entry : variant_folder) {
        if (entry.is_regular_file() && entry.path().extension() == ".variant") {
            std::filesystem::remove(entry.path());
        }
    }

    auto all_types = rttr::type::get_types();
    int variant_count = 0;

    for (const auto& type : all_types) {
        if (!type.is_valid() || 
            !type.is_derived_from<VariantBase>() ||
            type.get_name() == "VariantBase" ||
            type.is_pointer() ||
            type.is_wrapper()) {
            continue;
        }

        rttr_json::create_dummy(type);
        variant_count++;
    }
}

#endif // EDITOR_MODE
