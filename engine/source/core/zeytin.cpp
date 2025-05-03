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

#include "core/json/from_json.h"
#include "core/json/to_json.h"
#include "core/guid/guid.h"
#include "core/raylib_wrapper.h"
#include "core/utils.h"

#include "editor/editor_event.h"
#include "raylib.h"
#include "variant/variant_base.h"

#include "remote_logger/remote_logger.h"
#include "game/generated/rttr_registration.h" // required for registering types
#include "resource_manager/resource_manager.h"

#include "core/profiling.h"
#include "config_manager/config_manager.h""

Zeytin::Zeytin() {
#ifdef EDITOR_MODE
    m_editor_communication = std::make_unique<EditorCommunication>();
    subscribe_editor_events();
    while(!(m_editor_communication->is_connection_confirmed() && is_scene_ready())) {
        m_editor_communication->raise_events();
        begin_drawing(); // temp black screen, otherwise may crash
        clear_background(BLACK);
        end_drawing();
    }

    generate_variants();
    initial_sync_editor();
#else
    std::string startup_scene = CONFIG_GET("startup_scene", std::string, "main.scene");
    load_scene(ResourceManager::get().get_resource_subdir("scenes") / startup_scene);
    m_is_play_mode = true; // always set to play mode true if standalone
#endif

    m_render_texture = load_render_texture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

    m_camera.offset = {0,0},
    m_camera.target = {0, 0};
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;
}

Zeytin::~Zeytin() {
#ifdef EDITOR_MODE
    if(m_is_play_mode)
        exit_play_mode();  // for proper deinitialization
#endif
}

void Zeytin::run_frame() {

#ifdef EDITOR_MODE
    m_editor_communication->raise_events();
#endif

    begin_texture_mode(m_render_texture);
    clear_background(RAYWHITE);

    begin_mode2d(m_camera);

    post_init_variants();
    update_variants();

    end_mode2d();

    if(m_is_play_mode && !m_is_pause_play_mode) {
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

entity_id Zeytin::new_entity_id() {
    return generate_unique_id();
}

std::vector<rttr::variant>& Zeytin::get_variants(const entity_id& entity) {
    return m_storage[entity];
}

void Zeytin::clean_dead_variants() {
    for(auto& [entity_id, variants] : m_storage) {
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

std::string Zeytin::zserialize_entity(const entity_id id) {
    return rttr_json::serialize_entity(id, get_variants(id));
}

std::string Zeytin::zserialize_entity(const entity_id id, const std::filesystem::path& path) {
    return rttr_json::serialize_entity(id, get_variants(id), path);
}

entity_id Zeytin::zdeserialize_entity(const std::string& str) {
    entity_id id;
    std::vector<rttr::variant> variants;

    rttr_json::deserialize_entity(str, id, variants);

    auto& entity_variants = get_variants(id);
    entity_variants.clear();

    for (auto& var : variants) {
        VariantBase& base = var.get_value<VariantBase&>();
        base.on_init();
        entity_variants.push_back(std::move(var));
    }        
    return id;
}

std::string Zeytin::serialize_scene() {
    rapidjson::Document document;
    document.SetObject();

    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
    rapidjson::Value entitiesArray(rapidjson::kArrayType);

    for (const auto& [entity_id, variants] : m_storage) {
        std::string entityJson = zserialize_entity(entity_id);

        rapidjson::Document entityDoc;
        entityDoc.Parse(entityJson.c_str());

        if (!entityDoc.HasParseError()) {
            rapidjson::Value entityValue;
            entityValue.CopyFrom(entityDoc, allocator);
            entitiesArray.PushBack(entityValue, allocator);
        }
    }

    document.AddMember("type", "scene", allocator);
    document.AddMember("entities", entitiesArray, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return std::string(buffer.GetString(), buffer.GetSize());
}

void Zeytin::load_scene(const std::filesystem::path& path) {
    if (std::filesystem::exists(path)) {
        std::ifstream scene_file(path);
        std::string scene((std::istreambuf_iterator<char>(scene_file)),
                         std::istreambuf_iterator<char>());
        std::cout << scene << std::endl;
        scene_file.close();
        deserialize_scene(scene);
    }
    else {
        std::cerr << "Cannot load scene: Path " << path << " does not exist" << std::endl;
        exit(1);
    }


    std::cout << std::filesystem::current_path() << std::endl;
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();
    std::cout << buffer.str() << std::endl;
    deserialize_scene(buffer.str());
}

bool Zeytin::deserialize_scene(const std::string& scene) {
    m_storage.clear();

    rapidjson::Document scene_data;
    rapidjson::ParseResult parse_result = scene_data.Parse(scene.c_str());

    if (parse_result.IsError()) {
        log_error() << "Error parsing scene at offset " << parse_result.Offset() << std::endl;
        return false;
    }

    if (!scene_data.IsObject() || !scene_data.HasMember("type") ||
        !scene_data["type"].IsString() || strcmp(scene_data["type"].GetString(), "scene") != 0 ||
        !scene_data.HasMember("entities") || !scene_data["entities"].IsArray()) {
        log_error() << "Failed to deserialize scene: invalid scene format" << std::endl;
        return false;
    }

    const rapidjson::Value& entities = scene_data["entities"];

    for (rapidjson::SizeType i = 0; i < entities.Size(); i++) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        entities[i].Accept(writer);
        std::string entity_str = buffer.GetString();

        zdeserialize_entity(entity_str);
    }

    return true;
}

void Zeytin::post_init_variants() {
    ZPROFILE_ZONE_NAMED("Zeytin::post_init_variants()");

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            VariantBase& base = variant.get_value<VariantBase&>();
            if(base.is_dead || base.post_inited) continue;
            base.post_inited = true;
            {
                ZPROFILE_ZONE_NAMED("VariantBase::post_init_variants()");
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
            if(base.is_dead) continue;
            {
                ZPROFILE_ZONE_NAMED("VariantBase::on_update()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),base.get_type().get_name().to_string().size());
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
            if(base.is_dead) continue;
            {
                ZPROFILE_ZONE_NAMED("VariantBase::on_play_update()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                base.on_play_update();
            }
        }
    }
}

void Zeytin::play_start_variants() {
    ZPROFILE_ZONE_NAMED("Zeytin::play_start_variants()");

    if (m_started) return;
    m_started = true;

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            VariantBase& base = variant.get_value<VariantBase&>();
            if(base.is_dead) continue;
            {
                ZPROFILE_ZONE_NAMED("VariantBase::on_play_update()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                base.on_play_start();
            }
        }
    }
}

void Zeytin::play_late_start_variants() {
    ZPROFILE_ZONE_NAMED("Zeytin::play_late_start_variants()");

    if (m_late_started) return;
    m_late_started = true;

    for (auto& pair : m_storage) {   
        for (auto& variant : pair.second) {
            VariantBase& base = variant.get_value<VariantBase&>();
            if(base.is_dead) continue;
            {
                ZPROFILE_ZONE_NAMED("VariantBase::on_play_late_start()");
                ZPROFILE_TEXT(base.get_type().get_name().to_string().c_str(),base.get_type().get_name().to_string().size());
                ZPROFILE_VALUE(pair.first);
                base.on_play_late_start();
            }
        }
    }
}

void Zeytin::render() {
    draw_texture_pro(
        m_render_texture.texture,
        Rectangle{ 0, 0, (float)m_render_texture.texture.width, (float)-m_render_texture.texture.height },
        Rectangle{
        (get_screen_width() - VIRTUAL_WIDTH * ((get_screen_width() / VIRTUAL_WIDTH) < (get_screen_height() / VIRTUAL_HEIGHT) ? (get_screen_width() / VIRTUAL_WIDTH) : (get_screen_height() / VIRTUAL_HEIGHT))) * 0.5f,
        (get_screen_height() - VIRTUAL_HEIGHT * ((get_screen_width() / VIRTUAL_WIDTH) < (get_screen_height() / VIRTUAL_HEIGHT) ? (get_screen_width() / VIRTUAL_WIDTH) : (get_screen_height() / VIRTUAL_HEIGHT))) * 0.5f,
        VIRTUAL_WIDTH * ((get_screen_width() / VIRTUAL_WIDTH) < (get_screen_height() / VIRTUAL_HEIGHT) ? (get_screen_width() / VIRTUAL_WIDTH) : (get_screen_height() / VIRTUAL_HEIGHT)),
        VIRTUAL_HEIGHT * ((get_screen_width() / VIRTUAL_WIDTH) < (get_screen_height() / VIRTUAL_HEIGHT) ? (get_screen_width() / VIRTUAL_WIDTH) : (get_screen_height() / VIRTUAL_HEIGHT))
    },
    Vector2{ 0, 0 },
    0.0f,
    WHITE
    );
}

#ifdef EDITOR_MODE

void Zeytin::subscribe_editor_events() {
    EditorEventBus::get().subscribe<const std::string&>(
        EditorEvent::Scene, 
        [this](const auto& scene) {
            deserialize_scene(scene);
            m_is_scene_ready = true;
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
            m_is_pause_play_mode = true;
        }
    );

    EditorEventBus::get().subscribe<bool>(
        EditorEvent::UnPausePlayMode, 
        [this](bool) {
            m_is_pause_play_mode = false;
        }
    );

    EditorEventBus::get().subscribe<bool>(
        EditorEvent::Die, 
        [this](bool) {
            m_should_die = true;
        }
    );
}

void Zeytin::handle_entity_property_changed(const rapidjson::Document& doc) {
    assert(!doc.HasParseError());
    assert(doc.HasMember("entity_id"));
    assert(doc.HasMember("variant_type"));
    assert(doc.HasMember("key_type"));
    assert(doc.HasMember("key_path"));
    assert(doc.HasMember("value"));

    uint64_t entity_id = doc["entity_id"].GetUint64();
    const std::string& variant_type = doc["variant_type"].GetString();
    const std::string& key_type = doc["key_type"].GetString();
    const std::string& key_path = doc["key_path"].GetString();
    const std::string& value_str = doc["value"].GetString();

    if (m_storage.find(entity_id) == m_storage.end()) {
        log_error() << "Entity " << entity_id << " not found" << std::endl;
        return;
    }

    for (auto& variant : m_storage[entity_id]) {
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

            break; 
        }
    }
}

void Zeytin::handle_entity_variant_added(const rapidjson::Document& msg) {
    assert(!msg.HasParseError());
    assert(msg.HasMember("entity_id"));
    assert(msg.HasMember("variant_type"));

    entity_id entity_id = msg["entity_id"].GetUint64();
    auto& variants = m_storage[entity_id];

    VariantCreateInfo info;
    info.entity_id = entity_id;
    std::vector<rttr::argument> args;
    args.push_back(info);

    rttr::type rttr_type = rttr::type::get_by_name(msg["variant_type"].GetString());
    
    if(!rttr_type.is_valid()) {
        log_error() << "Variant type is invalid: " << rttr_type.get_name() << std::endl;
        return;
    }

    rttr::variant obj = rttr_type.create(args);

    variants.push_back(std::move(obj));
}

void Zeytin::handle_entity_variant_removed(const rapidjson::Document& msg) {
    assert(!msg.HasParseError());
    assert(msg.HasMember("entity_id"));
    assert(msg.HasMember("variant_type"));

    entity_id entity_id = msg["entity_id"].GetUint64();
    auto& variants = m_storage[entity_id];
    rttr::type rttr_type = rttr::type::get_by_name(msg["variant_type"].GetString());

    if(!rttr_type.is_valid()) {
        log_error() << "Variant type is invalid: " << rttr_type.get_name() << std::endl;
        return;
    }

    remove_variant(entity_id, rttr_type);
}

void Zeytin::remove_variant(entity_id id, const rttr::type& type) {
    for(auto& variant : get_variants(id)) {
        if(variant.get_type() == type) {
            VariantBase& base = variant.get_value<VariantBase&>();
            base.is_dead = true;
        }
    }
}

void Zeytin::remove_entity(entity_id id) {
    m_storage[id].clear();
}


void Zeytin::handle_entity_removed(const rapidjson::Document& msg) {
    assert(!msg.HasParseError());
    assert(msg.HasMember("entity_id"));

    entity_id entity_id = msg["entity_id"].GetUint64();

    remove_entity(entity_id);
}


void Zeytin::enter_play_mode(bool is_paused) {
    if(m_is_play_mode) return;

    std::string scene = serialize_scene();

    std::filesystem::create_directory("temp");
    std::ofstream scene_file("temp/backup.scene");
    scene_file << scene;
    scene_file.close();

    m_is_pause_play_mode = is_paused;
    m_is_play_mode = true;
}

void Zeytin::exit_play_mode() {
    m_storage.clear();
    m_started = false;
    m_is_play_mode = false;

    if (std::filesystem::exists("temp/backup.scene")) {
        std::ifstream scene_file("temp/backup.scene");
        std::string scene((std::istreambuf_iterator<char>(scene_file)),
                         std::istreambuf_iterator<char>());
        scene_file.close();
        deserialize_scene(scene);
        std::filesystem::remove_all("temp");
    }
    else {
        log_error() << "Cannot exit playmode because scene backup is not found" << std::endl;
        exit(1);
    }
}

void Zeytin::initial_sync_editor() {
    std::string scene = serialize_scene();
    EditorEventBus::get().publish<std::string>(EditorEvent::SyncEditor, scene);
}

void Zeytin::sync_editor() {
    static float sync_timer = 0.0f;
    static const float SYNC_INTERVAL = 0.1f;
    
    sync_timer += get_frame_time(); 
    
    if (sync_timer >= SYNC_INTERVAL) {
        sync_timer = 0.0f; 
        
        std::string scene = serialize_scene();
        EditorEventBus::get().publish<std::string>(EditorEvent::SyncEditor, scene);
    }
}


void Zeytin::generate_variants() {
    for(const auto& entry : ResourceManager::get().get_variant_folder()) {
        if(entry.is_regular_file() && entry.path().extension() == ".variant") {
            std::filesystem::remove(entry.path());
        }
    }

    auto all_types = rttr::type::get_types();

    std::vector<rttr::type> valid_types;

    for (const auto& type : all_types) {
        if (!type.is_valid()) {
            std::cerr << "Type is not valid: " << type.get_name() << std::endl;
            continue;
        }

        if (!type.is_derived_from<VariantBase>() ||
            type.get_name() == "VariantBase" ||
            type.is_pointer() ||
            type.is_wrapper()) {

            continue;
        }

        valid_types.push_back(type);
    }

    for (const auto& type : valid_types) {
        std::cout << "Generating variant for: " << type.get_name() << std::endl;
        rttr_json::create_dummy(type);
    }
}

void Zeytin::generate_variant(const rttr::type& type) {
    rttr_json::create_dummy(type);
}

#endif 
