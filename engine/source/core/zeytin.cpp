module;

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include "core/profiling.h"
#include "raylib.h"
#include "rttr/variant.h"

module zeytin.zeytin;
import zeytin.level;
import zeytin.resource;
import zeytin.logger;
import zeytin.json;
import zeytin.component;
import zeytin.guid;
import zeytin.property;
import zeytin.raylib;
import zeytin.editor.communication;
import zeytin.shared_texture;

#ifdef EDITOR_MODE
import zeytin.manipulator.manager;
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
    auto& editor_communication = EditorCommunication::get();
    subscribe_editor_events();

    // Initialize manipulator manager singleton
    ManipulatorManager::get().initialize();

    // init shared texture for editor viewport
    if (!m_shared_texture_writer.initialize()) {
        log_error("Failed to initialize shared texture writer");
    }

    // wait for editor connection to be established
    while (!editor_communication.is_connection_confirmed() || !m_state.scene_ready) {
        editor_communication.raise_events();

        // black screen while waiting for connection
        begin_drawing();
        clear_background(BLACK);
		draw_text("Connecting to the editor...", 0, 0, 20, WHITE);
        end_drawing();
    }
}
#else

void Zeytin::initialize_standalone() {
    std::string default_level = "entry"; 
    std::string scene_json = LevelManager::load_level(default_level);
    
    if (scene_json.empty() || !deserialize_scene(scene_json)) {
        log_error() << "Failed to load startup level: " << default_level << std::endl;
        m_state.should_die = true;
        return;
    }
    
    m_current_level_name = default_level;  
    m_state.play_mode = true;
}

#endif 

void Zeytin::shutdown() {

#ifdef EDITOR_MODE
    if (m_state.play_mode) {
        exit_play_mode();
    }

    EditorCommunication::get().shutdown();
#endif

    if (m_render_texture.id != 0) {
        unload_render_texture(m_render_texture);
        m_render_texture.id = 0;
    }
    
    m_storage.clear();
}

EntityID Zeytin::new_entity() {
	return generate_unique_id(); 
}

void Zeytin::run_frame() {
    ZPROFILE_FUNCTION();
    
#ifdef EDITOR_MODE
    EditorCommunication::get().raise_events();
#endif

	if(m_state.load_level_next_frame) {
        if (!m_pending_level_name.empty()) {
            std::string scene_json = LevelManager::load_level(m_pending_level_name);
            
            if (!scene_json.empty()) {
                m_storage.clear();
                
                if (deserialize_scene(scene_json)) {
                    m_state.started = false;
                    m_state.late_started = false;
					m_current_level_name = m_pending_level_name;  
                    //log_info() << "Successfully loaded level: " << m_pending_level_name << std::endl;
                } else {
                    log_error("Failed to deserialize level: {}", m_pending_level_name);
                }
            } else {
                log_error("Failed to load level: {}", m_pending_level_name);
            }
            
            m_pending_level_name.clear();
        }
        m_state.load_level_next_frame = false;
    }

	if(m_state.reload_next_frame) {
#ifdef EDITOR_MODE
    if (!m_current_level_name.empty()) {
        request_level_load(m_current_level_name);
    } else {
        exit_play_mode();
        enter_play_mode(false);
    }
#else
    // Standalone: reload current level
    if (!m_current_level_name.empty()) {
        request_level_load(m_current_level_name);
    } else {
        //log_error() << "No level to reload" << std::endl;
    }
#endif
    m_state.reload_next_frame = false;
}

    begin_texture_mode(m_render_texture);
    clear_background(BLACK);

    begin_mode2d(m_camera);
    
    update_components();
    
    if (m_state.play_mode && !m_state.pause_play_mode) {
        clean_dead_variants();
        play_start_components();
        play_update_components();
        play_late_update_components();
    }

	// call to manipulator
	// TODO: come up with a callback register system ? instead of putting here ?
	ManipulatorManager::get().handle_keyboard_shortcuts();
	ManipulatorManager::get().handle_selected(m_selected_entity);

    end_mode2d();
    end_texture_mode();

#ifdef EDITOR_MODE
    // write render texture to shared memory for editor viewport
    if (m_shared_texture_writer.is_initialized()) {
        Image image = LoadImageFromTexture(m_render_texture.texture);
        // flip vertically since OpenGL textures are upside down
        ImageFlipVertical(&image);
        m_shared_texture_writer.write_pixels(
            static_cast<const unsigned char*>(image.data),
            static_cast<uint32_t>(image.width),
            static_cast<uint32_t>(image.height)
        );
        UnloadImage(image);
    }
#endif

    begin_drawing();
    clear_background(BLACK);
    render();
    end_drawing();
}

ComponentList& Zeytin::get_components(const EntityID& entity) {
    return m_storage[entity];
}

void Zeytin::remove_entity(EntityID id) {
	m_storage.erase(id);
}


void Zeytin::clean_dead_variants() {
    ZPROFILE_FUNCTION();
    
    for (auto& [EntityID, variants] : m_storage) {
        variants.erase(
            std::remove_if(variants.begin(), variants.end(),
                [](rttr::variant& variant) {
                    Component& var_base = variant.get_value<Component&>();
                    return var_base.is_dead;
                }
            ),
            variants.end()
        );
    }
}

void Zeytin::clean_dead_variants(const EntityID entity) {
    ZPROFILE_FUNCTION();
	auto& components = get_components(entity);
 	components.erase(
            std::remove_if(components.begin(), components.end(),
                [](rttr::variant& variant) {
                    Component& var_base = variant.get_value<Component&>();
                    return var_base.is_dead;
                }
            ),
            components.end()
        );
}

std::string Zeytin::serialize_entity(const EntityID id) {
    return rttr_json::serialize_entity(id, get_components(id));
}

std::string Zeytin::serialize_entity(const EntityID id, const std::filesystem::path& path) {
    return rttr_json::serialize_entity(id, get_components(id), path);
}

EntityID Zeytin::deserialize_entity(const std::string& entity_json) {
    if (entity_json.empty()) {
        //log_error() << "Cannot deserialize empty entity JSON" << std::endl;
        return 0;
    }
    
    EntityID id;
    std::vector<rttr::variant> variants;

    if (!rttr_json::deserialize_entity(entity_json, id, variants)) {
        //log_error() << "Failed to deserialize entity" << std::endl;
        return 0;
    }

    auto& entity_variants = get_components(id);
    entity_variants.clear();

    for (auto& var : variants) {
        Component& base = var.get_value<Component&>();
        base.on_init();
        entity_variants.push_back(std::move(var));
    }
    
    return id;
}

void Zeytin::remove_variant(EntityID id, const rttr::type& type) {
    auto& variants = get_components(id);
    for (auto& variant : variants) {
        if (variant.get_type() == type) {
            Component& base = variant.get_value<Component&>();
			log_trace("!!!!!!!!!!!!!!!!!!!Removed component {} from entity {}", type.get_name().data(), base.get_id());
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

    for (const auto& [EntityID, variants] : m_storage) {
        std::string entity_json = serialize_entity(EntityID);
        if (entity_json.empty()) {
            continue;
        }

        rapidjson::Document entity_doc;
        rapidjson::ParseResult parse_result = entity_doc.Parse(entity_json.c_str());
        
        if (parse_result.IsError()) {
            //log_error() << "Failed to parse entity JSON for entity: " << EntityID << std::endl;
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
        log_error("Scene file does not exist: {}", path.string());
        return false;
    }

    std::ifstream scene_file(path);
    if (!scene_file.is_open()) {
        log_error("Failed to open scene file: {}", path.string());
        return false;
    }

    std::stringstream buffer;
    buffer << scene_file.rdbuf();
    scene_file.close();
    
    std::string scene_content = buffer.str();
    if (scene_content.empty()) {
        log_error("Scene file is empty: {}", path.string());
        return false;
    }
    
    return deserialize_scene(scene_content);
}

bool Zeytin::deserialize_scene(const std::string& scene) {
    ZPROFILE_FUNCTION();
    
    if (scene.empty()) {
        log_error("Cannot deserialize empty scene");
        return false;
    }
    
    m_storage.clear();
    
    m_state.started = false;
    m_state.late_started = false;

    rapidjson::Document scene_data;
    rapidjson::ParseResult parse_result = scene_data.Parse(scene.c_str());

    if (parse_result.IsError()) {
        //log_error() << "Error parsing scene at offset " << parse_result.Offset() << std::endl;
        return false;
    }

    if (!scene_data.IsObject() || 
        !scene_data.HasMember("type") ||
        !scene_data["type"].IsString() || 
        strcmp(scene_data["type"].GetString(), "scene") != 0 ||
        !scene_data.HasMember("entities") || 
        !scene_data["entities"].IsArray()) {
        
        log_error("Invalid scene format");
        return false;
    }

    const rapidjson::Value& entities = scene_data["entities"];
    int successful_entities = 0;
    
    for (rapidjson::SizeType i = 0; i < entities.Size(); i++) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        entities[i].Accept(writer);
        
        const std::string entity_str = buffer.GetString();
        const EntityID entity_id = deserialize_entity(entity_str);
        
        if (entity_id != 0) {
            successful_entities++;
        }
    }

    if (successful_entities == 0 && entities.Size() > 0) {
        log_error("Failed to deserialize any entities from scene");
        return false;
    }

#ifdef EDITOR_MODE
	// NOTE: this might need more testing
	initial_sync_editor();
#endif

    //log_info() << "Scene loaded with " << successful_entities << " entities" << std::endl;
    return true;
}

bool Zeytin::switch_to_level(const std::string& level_name) {
    std::string scene_json = LevelManager::load_level(level_name);
    
    if (scene_json.empty()) {
        //log_error() << "Failed to load level: " << level_name << std::endl;
        return false;
    }
    
    // Clear current scene
    m_storage.clear();
    
    // Load new scene
    if (!deserialize_scene(scene_json)) {
        //log_error() << "Failed to deserialize level: " << level_name << std::endl;
        return false;
    }
    
    // Reset state
    m_state.started = false;
    m_state.late_started = false;
    
    //log_info() << "Switched to level: " << level_name << std::endl;
    return true;
}

void Zeytin::update_components() {
    ZPROFILE_ZONE_NAMED("Zeytin::update_variants()");

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            Component& base = variant.get_value<Component&>();
            
            if (base.is_dead) {
                continue;
            }
            
            {
                ZPROFILE_ZONE_NAMED("Component::on_update()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),
                              base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                
                base.on_update();
            }
        }
    }
}

void Zeytin::play_update_components() {
    ZPROFILE_ZONE_NAMED("Zeytin::play_update_variants()");

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            Component& base = variant.get_value<Component&>();
            
            if (base.is_dead) {
                continue;
            }
            
            {
                ZPROFILE_ZONE_NAMED("Component::on_play_update()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),
                              base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                
                base.on_play_update();
            }
        }
    }
}
void Zeytin::play_late_update_components() {
    ZPROFILE_ZONE_NAMED("Zeytin::play_late_update_variants()");

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            Component& base = variant.get_value<Component&>();
            
            if (base.is_dead) {
                continue;
            }
            
            {
                ZPROFILE_ZONE_NAMED("Component::on_play_late_update()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),
                              base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                
                base.on_play_late_update();
            }
        }
    }
}

void Zeytin::play_start_components() {
    ZPROFILE_ZONE_NAMED("Zeytin::play_start_variants()");

    if (m_state.started) {
        return;
    }
    
    m_state.started = true;

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            Component& base = variant.get_value<Component&>();
            
            if (base.is_dead) {
                continue;
            }
            
            {
                ZPROFILE_ZONE_NAMED("Component::on_play_start()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),
                              base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                
                base.on_play_start();
            }
        }
    }
}

void Zeytin::render() {
    ZPROFILE_FUNCTION();
    
    const float screen_width = get_screen_width();
    const float screen_height = get_screen_height();
    
    const float scale_factor = (screen_width / VIRTUAL_WIDTH) < (screen_height / VIRTUAL_HEIGHT) ? 
                         (screen_width / VIRTUAL_WIDTH) : (screen_height / VIRTUAL_HEIGHT);
    
    const float render_width = VIRTUAL_WIDTH * scale_factor;
    const float render_height = VIRTUAL_HEIGHT * scale_factor;
    
    const float pos_x = (screen_width - render_width) * 0.5f;
    const float pos_y = (screen_height - render_height) * 0.5f;

    draw_texture_pro(
        m_render_texture.texture,
        {0, 0, (float)m_render_texture.texture.width, (float)-m_render_texture.texture.height},
        {pos_x, pos_y, render_width, render_height},
        {0, 0},
        0.0f,
        WHITE
    );
}

void Zeytin::request_level_load(const std::string& level_name) {
    m_pending_level_name = level_name;
    m_state.load_level_next_frame = true;
    log_info("Level load requested: {}", level_name);
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
        EditorEvent::EntityAdded, 
        [this](const rapidjson::Document& msg) {
            handle_entity_added(msg);
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

    EditorEventBus::get().subscribe<const rapidjson::Document&>(
        EditorEvent::WindowStateChanged,
        [](const rapidjson::Document& doc) {
            if (doc.HasMember("hidden")) {
                bool hidden = doc["hidden"].GetBool();
                if (hidden) {
                    SetWindowState(FLAG_WINDOW_HIDDEN);
                } else {
                    ClearWindowState(FLAG_WINDOW_HIDDEN);
                }
            }
        }
    );

    EditorEventBus::get().subscribe<const rapidjson::Document&>(
        EditorEvent::EntitySelected,
        [this](const rapidjson::Document& msg) {
            handle_entity_selected(msg);
        }
    );
}

void Zeytin::handle_entity_property_changed(const rapidjson::Document& doc) {
    // Validate document structure
    if (doc.HasParseError()) {
        log_error("JSON parse error in property change document");
        return;
    }
    
    if (!doc.HasMember("entity_id")) {
        log_error("[handle_entity_property_changed] Missing required field: entity_id");
        return;
    }
    if (!doc.HasMember("variant_type")) {
        log_error("[handle_entity_property_changed] Missing required field: variant_type");
        return;
    }
    if (!doc.HasMember("key_type")) {
        log_error("[handle_entity_property_changed] Missing required field: key_type");
        return;
    }
    if (!doc.HasMember("key_path")) {
        log_error("[handle_entity_property_changed] Missing required field: key_path");
        return;
    }
    if (!doc.HasMember("value")) {
        log_error("[handle_entity_property_changed] Missing required field: value");
        return;
    }

    
    const uint64_t entity_id = doc["entity_id"].GetUint64();
    const std::string variant_type = doc["variant_type"].GetString();
    const std::string key_type = doc["key_type"].GetString();
    const std::string key_path = doc["key_path"].GetString();
    const std::string value_str = doc["value"].GetString();

    log_trace("[handle_entity_property_changed] {} {} {} {}", entity_id, variant_type, key_type, key_path, value_str);
    
    auto& variants = get_components(entity_id);
    
    for (auto& variant : variants) {
        if (variant.get_type().get_name() == variant_type) {
            const std::vector<std::string> path_parts = split_path(key_path);
            
            if (path_parts.empty()) {
                log_error("Empty key path for entity {} variant {}", entity_id, variant_type);
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
                log_error("Unsupported key_type '{}' for entity {} variant {} property {}", 
                         key_type, entity_id, variant_type, key_path);
                return;
            }
            
            return;
        }
    }
    
    log_error("Variant '{}' not found on entity {}", variant_type, entity_id);
}

void Zeytin::handle_entity_variant_added(const rapidjson::Document& msg) {
    if (msg.HasParseError() || 
        !msg.HasMember("entity_id") || 
        !msg.HasMember("variant_type")) {
        
        log_error("Invalid variant add document format");
        return;
    }

    const EntityID entity_id = msg["entity_id"].GetUint64();
    const char* variant_type_name = msg["variant_type"].GetString();

    const rttr::type rttr_type = rttr::type::get_by_name(variant_type_name);
	assert(rttr_type.is_valid());

	clean_dead_variants(entity_id);
    auto& variants = get_components(entity_id);

	 for (const auto& existing : variants) {
		if (existing.get_type() == rttr_type) {
			log_warning("Entity with ID {} already has component of type {}", entity_id, rttr_type.get_name().to_string());
			return;
		}
	}

    rttr::variant obj;
    
    // if component_data is provided, deserialize from it
    if (msg.HasMember("component_data")) {
        const rapidjson::Value& component_data = msg["component_data"];
        
        // convert component_data to JSON string
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        component_data.Accept(writer);
        obj = rttr_json::deserialize_component(entity_id, buffer.GetString());
    } else {
        // create with defaults
        obj = rttr_type.create();
        assert(obj.is_valid());
        rttr_type.set_property_value("entity_id", obj, entity_id);
    }
    
    Component& base = obj.get_value<Component&>();
    base.on_init();
    variants.push_back(std::move(obj));
}

void Zeytin::handle_entity_variant_removed(const rapidjson::Document& msg) {
    if (msg.HasParseError() || 
        !msg.HasMember("entity_id") || 
        !msg.HasMember("variant_type")) {
        
        log_error("Invalid variant remove document format");
        return;
    }

    const EntityID EntityID = msg["entity_id"].GetUint64();
    const char* variant_type_name = msg["variant_type"].GetString();

    rttr::type rttr_type = rttr::type::get_by_name(variant_type_name);
    if (!rttr_type.is_valid()) {
        log_error("Invalid variant type: {}", variant_type_name);
        return;
    }

    remove_variant(EntityID, rttr_type);
    log_trace("Removed variant {}, from entity {}", variant_type_name, EntityID);
}

void Zeytin::handle_entity_added(const rapidjson::Document& msg) {
    if (msg.HasParseError() || 
        !msg.HasMember("entity_id") || 
        !msg.HasMember("entity_data")) {
        log_error("Invalid entity add document format");
        return;
    }

    const EntityID entity_id = msg["entity_id"].GetUint64();
    
    // Check if entity already exists
    if (m_storage.find(entity_id) != m_storage.end()) {
        log_warning("Entity with ID {} already exists, removing the entity first...", entity_id);
		remove_entity(entity_id);
    }

    // Serialize entity_data back to JSON string
    const rapidjson::Value& entity_data = msg["entity_data"];
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    entity_data.Accept(writer);
    std::string entity_json(buffer.GetString(), buffer.GetSize());

    // Deserialize the entity (this will add it to storage)
    EntityID deserialized_id = deserialize_entity(entity_json);
    
    if (deserialized_id == 0) {
        log_error("Failed to deserialize entity with ID {}", entity_id);
        return;
    }
    
    if (deserialized_id != entity_id) {
        log_warning("Deserialized entity ID {} doesn't match expected ID {}", 
                    deserialized_id, entity_id);
    }

    log_info("Added entity {}", entity_id);
}

void Zeytin::handle_entity_removed(const rapidjson::Document& msg) {
    if (msg.HasParseError() || !msg.HasMember("entity_id")) {
        log_error("Invalid entity remove document format");
        return;
    }

    EntityID EntityID = msg["entity_id"].GetUint64();

    remove_entity(EntityID);
    log_info("Removed entity {}", EntityID);
}

void Zeytin::enter_play_mode(bool is_paused) {
    if (m_state.play_mode) {
        return;
    }

    std::string scene = serialize_scene();
    if (scene.empty()) {
        log_error("Failed to serialize scene for play mode");
        return;
    }

    std::filesystem::create_directory("temp");
    std::ofstream scene_file("temp/backup.scene");
    if (!scene_file.is_open()) {
        log_error("Failed to create backup scene file");
        return;
    }
    
    scene_file << scene;
    scene_file.close();

    m_state.pause_play_mode = is_paused;
    m_state.play_mode = true;
    
	log_info("Entered play mode");
}

void Zeytin::exit_play_mode() {
    m_state.started = false;
    m_state.late_started = false;
    m_state.play_mode = false;
    m_state.pause_play_mode = false;
	m_pending_level_name = "";

    std::filesystem::path backup_path = "temp/backup.scene";
    if (!std::filesystem::exists(backup_path)) {
        log_error("Cannot exit play mode: scene backup not found");
        return;
    }

    m_storage.clear();

    if (!load_scene(backup_path)) {
        log_error("Failed to load scene from backup");
        return;
    }

    std::filesystem::remove_all("temp");
    
    log_info("Exited play mode");
}

void Zeytin::initial_sync_editor() {
    const std::string scene = serialize_scene();
    if (!scene.empty()) {
        EditorEventBus::get().publish<std::string>(EditorEvent::SyncEditor, scene);
        log_info("Initial scene sync with editor");
    } else {
        log_error("Failed to serialize scene for initial sync");
    }
}

void Zeytin::sync_editor() {
    static float sync_timer = 0.0f;
    static const float SYNC_INTERVAL = 0.1;
    
    sync_timer += get_frame_time();
    
    if (sync_timer >= SYNC_INTERVAL) {
        sync_timer = 0.0f;
        
        std::string scene = serialize_scene();
        if (!scene.empty()) {
            EditorEventBus::get().publish<std::string>(EditorEvent::SyncEditor, scene);
        }
    }
}

void Zeytin::handle_entity_selected(const rapidjson::Document& msg) {
    if (msg.HasParseError() || !msg.HasMember("entity_id")) {
        log_error("Invalid entity selection message");
        m_selected_entity = 0;
        return;
    }

    m_selected_entity = msg["entity_id"].GetUint64();
    log_info("Entity selected: {}", m_selected_entity);
}

#endif // EDITOR_MODE
