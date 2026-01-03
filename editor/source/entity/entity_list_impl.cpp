module;

#include <filesystem>
#include <vector>
#include <optional>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"

module zeytin.entity.list;
import zeytin.resource;
import zeytin.engine.event;
import zeytin.logger;

namespace {
    constexpr const char* BACKUP_DIR = "temp_backup";
    constexpr const char* ENTITY_EXTENSION = ".entity";
}

EntityList::EntityList() {
    auto levels = get_available_levels();
    if (!levels.empty()) {
        load_level(levels[0]);
    } else {
        auto default_level_path = ResourceManager::get().get_resource_subdir("levels") / "default";
        std::filesystem::create_directories(default_level_path);
        load_level(Level("default", default_level_path));
    }
    
    register_event_handlers();
}

void EntityList::register_event_handlers() {
    EngineEventBus::get().subscribe<bool>(
        EngineEvent::EngineStarted,
        [this](auto _) {
            std::string scene = as_string();
            if(!scene.empty()) {
                EngineEventBus::get().publish<const std::string&>(EngineEvent::EngineSendScene, scene);
            }
            else {
                //log_warning() << "Empty scene will not be sent to the engine" << std::endl;
            }
        }
    );

    EngineEventBus::get().subscribe<std::string>(
    EngineEvent::SyncEditor,
        [this](auto msg) {
            if(!should_sync_runtime()) {
                log_warning("EDITOR: Sync received but ignored");
                return;
            }

            rapidjson::Document doc;
            doc.Parse(msg.c_str());

            if(doc.HasParseError()) {
                return;
            }

            sync_entities_from_document(doc);
            if(!m_is_synced_once) {
                m_is_synced_once = true;
                //log_info() << "Initial sync with runtime" << std::endl;
            }
        }
    );

    
    EngineEventBus::get().subscribe<bool>(
        EngineEvent::EnterPlayMode,
        [this](auto) {
            m_is_play_mode = true;
            backup_entities();
        }
    );
    
    EngineEventBus::get().subscribe<bool>(
        EngineEvent::ExitPlayMode,
        [this](bool) {
            m_is_play_mode = false;
            load_entities(BACKUP_DIR);
            clean_backup_entities();
        }
    );

    EngineEventBus::get().subscribe<bool>(
        EngineEvent::EngineStopped,
        [this](bool) {
            m_is_play_mode = false;
            m_is_synced_once = false;
        }
    );
}

std::string EntityList::as_string() const {
    rapidjson::Document document;
    document.SetObject();
    auto& allocator = document.GetAllocator();

    document.AddMember("type", "scene", allocator);
    
    rapidjson::Value entities_array(rapidjson::kArrayType);
    
    for (const auto& entity : m_entities) {
        if (entity.is_dead()) {
            continue;
        }
        
        std::string entity_str = entity.as_string();

        if(entity_str.empty()) {
            //log_error() << "Failed to get entity as string: " << entity.get_name() << std::endl;
            continue;
        }
        
        rapidjson::Document entity_doc;
        rapidjson::ParseResult parse_result = entity_doc.Parse(entity_str.c_str());

        if (parse_result.IsError()) {
             //log_error() << "JSON parse error: " << parse_result.Code()
             //         << " (offset: " << parse_result.Offset() << ")" << std::endl;
            continue;
        }
        
        rapidjson::Value entity_value;
        entity_value.CopyFrom(entity_doc, allocator);
        entities_array.PushBack(entity_value, allocator);
    }
    
    document.AddMember("entities", entities_array, allocator);
    
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    if (!document.Accept(writer)) {
        //log_error() << "Failed to write document to string buffer" << std::endl;
        return "";
    }
    
    return std::string(buffer.GetString(), buffer.GetSize());
}

void EntityList::sync_entities_from_document(const rapidjson::Document& document) {
    if(!document.IsObject()) {
        return;
    }

    if (!document.HasMember("entities")) {
        log_error("Received sync message does not have 'entities' member");
        return;
    }
    
    if (!document["entities"].IsArray()) {
        log_error("Received sync message 'entities' is not an array");
        return;
    }
    
    const auto& entities = document["entities"].GetArray();
    for (const auto& entity : entities) {
        if (!entity.HasMember("entity_id") || !entity["entity_id"].IsUint64()) {
            log_error("Entity missing required 'entity_id' field");
            continue;
        }
        
        if (!entity.HasMember("variants") || !entity["variants"].IsArray()) {
            log_error("Entity missing required 'variants' field");
            continue;
        }

        uint64_t entity_id = entity["entity_id"].GetUint64();
        bool found = false;
        
        for (auto& entity_doc : m_entities) {
            auto& existing_doc = entity_doc.get_document();
            if (existing_doc.HasMember("entity_id") &&
                existing_doc["entity_id"].GetUint64() == entity_id) {

				existing_doc["variants"].Clear();

                rapidjson::Document new_doc;
                new_doc.CopyFrom(entity, new_doc.GetAllocator());
                entity_doc.set_document(std::move(new_doc));
                found = true;
                break;
            }
        }
        
        if (!found) {
            //log_error() << "Entity with ID " << entity_id << " not found in entity list" << std::endl;
        }
    }
}

void EntityList::backup_entities() {
    std::filesystem::path backupDir = BACKUP_DIR;

    std::error_code ec;
    if (!std::filesystem::exists(backupDir)) {
        if (!std::filesystem::create_directory(backupDir, ec)) {
            //log_error() << "Failed to create backup directory: " << ec.message() << std::endl;
            return;
        }
    }

    for (const auto& entry : std::filesystem::directory_iterator(backupDir, ec)) {
        if (ec) {
            //log_error() << "Error iterating backup directory: " << ec.message() << std::endl;
            return;
        }

        if (!std::filesystem::remove(entry.path(), ec)) {
            //log_error() << "Failed to remove old backup file: " << entry.path() << ", error: " << ec.message() << std::endl;
        }
    }

    for (const auto& entity : m_entities) {
        if(entity.is_dead()) continue;

        std::string name = entity.get_name();

        if(name.empty()) {
            //log_error() << "Attempt to process unnamed entity" << std::endl;
            continue;
        }

        std::filesystem::path backupPath = backupDir / (name + ENTITY_EXTENSION);

        entity.save_to_file(backupPath);
    }
}

void EntityList::clean_backup_entities() {
    std::filesystem::remove_all(BACKUP_DIR);;
}

void EntityList::load_entities(const std::filesystem::path& path) {
    m_entities.clear();

    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        if (ec) {
            //log_error() << "Error checking if path exists: " << path << ", error: " << ec.message() << std::endl;
            return;
        }

        //log_error() << "Path does not exist: " << path << std::endl;
        return;
    }

    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path, ec)) {
        if (ec) {
            //log_error() << "Error iterating directory: " << path << ", error: " << ec.message() << std::endl;
            return;
        }

        if (!entry.is_regular_file(ec) || entry.path().extension() != ENTITY_EXTENSION) {
            if (ec) {
                //log_error() << "Error checking if path is regular file: " << entry.path() << ", error: " << ec.message() << std::endl;
            }
            continue;
        }

        std::filesystem::path file_path = entry.path();
        std::string file_name = file_path.stem().string();

        auto& entity = m_entities.emplace_back(EntityDocument(std::move(file_name)));
        entity.load_from_file(file_path);
    }
}

void EntityList::load_entity_from_file(const std::filesystem::path& path) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || !std::filesystem::is_regular_file(path, ec)) {
        if (ec) {
            //log_error() << "Error checking path: " << path << ", error: " << ec.message() << std::endl;
        } else {
            //log_error() << "Path does not exist or is not a regular file: " << path << std::endl;
        }
        return;
    }

    std::string file_name = path.stem().string();

    if(file_name.empty()) {
        //log_error() << "Error: Loading unnamed entity is not allowed" << std::endl;
        return;
    }

    auto& entity = m_entities.emplace_back(EntityDocument(std::move(file_name)));

    entity.load_from_file(path);
}

std::optional<std::reference_wrapper<EntityDocument>> EntityList::find_entity_by_id(uint64_t entity_id) {
    auto it = std::ranges::find_if(m_entities.begin(), m_entities.end(), 
        [entity_id](const EntityDocument& entity) {
            return entity.get_id() == entity_id;
        }
    );
    
    if (it == m_entities.end()) {
        log_error("EntityList: Entity with ID {} not found", entity_id);
        return std::nullopt;
    }
    
    if (!it->is_valid()) {
        log_error("EntityList: Entity with ID {} is invalid", entity_id);
        return std::nullopt;
    }
    
    return std::ref(*it);
}

void EntityList::load_level(const Level& level) {
    if (!level.is_valid()) {
        //log_error() << "Invalid level" << std::endl;
        return;
    }
    
    m_current_level = level;
    load_entities(level.path); // reuse existing function
    
    // Sync with engine if running
    if (m_is_play_mode || m_is_synced_once) {
        std::string scene = as_string();
        EngineEventBus::get().publish<const std::string&>(EngineEvent::EngineSendScene, scene);
    }
}

std::vector<Level> EntityList::get_available_levels() const {
    std::vector<Level> levels;
    auto levels_path = ResourceManager::get().get_resource_subdir("levels");
    
    for (const auto& entry : std::filesystem::directory_iterator(levels_path)) {
        if (entry.is_directory()) {
            levels.emplace_back(entry.path().filename().string(), entry.path());
        }
    }
    return levels;
}

void EntityList::save_all_entities() {
    if (!m_current_level.is_valid()) {
        //log_error() << "Cannot save entities: current level is invalid" << std::endl;
        return;
    }

    for (auto& entity : m_entities) {
        if (!entity.is_dead()) {
            std::filesystem::path entity_path = m_current_level.path / (entity.get_name() + ".entity");
            entity.save_to_file(entity_path);
        } else {
            std::filesystem::path entity_path = m_current_level.path / (entity.get_name() + ".entity");
            std::filesystem::remove(entity_path);
        }
    }
}
