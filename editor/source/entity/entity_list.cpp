#include "entity/entity_list.h"

#include <filesystem>
#include <mutex>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include "logger.h"
#include "engine/engine_event.h"
#include "resource_manager/resource_manager.h"

#include "common/global_mutexes.h"

namespace {
    constexpr const char* BACKUP_DIR = "temp_backup";
    constexpr const char* ENTITY_EXTENSION = ".entity";
}

EntityList::EntityList() {
    load_entities(ResourceManager::get().get_entities_path());
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
                log_warning() << "Empty scene will not be sent to the engine" << std::endl;
            }
        }
    );

    EngineEventBus::get().subscribe<std::string>(
    EngineEvent::SyncEditor,
        [this](auto msg) {
            if(!should_sync_runtime()) {
                log_warning() << "EDITOR: Sync received but ignored" << std::endl;
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
                log_info() << "Initial sync with runtime" << std::endl;
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
             log_error() << "Failed to get entity as string: " << entity.get_name() << std::endl;
            continue;
        }
        
        rapidjson::Document entity_doc;
        rapidjson::ParseResult parse_result = entity_doc.Parse(entity_str.c_str());

        if (parse_result.IsError()) {
             log_error() << "JSON parse error: " << parse_result.Code()
                      << " (offset: " << parse_result.Offset() << ")" << std::endl;
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
        log_error() << "Failed to write document to string buffer" << std::endl;
        return "";
    }
    
    return std::string(buffer.GetString(), buffer.GetSize());
}

void EntityList::sync_entities_from_document(const rapidjson::Document& document) {
    if(!document.IsObject()) {
        return;
    }

    if (!document.HasMember("entities")) {
        log_error() << "Received sync message does not have 'entities' member" << std::endl;
        return;
    }
    
    if (!document["entities"].IsArray()) {
        log_error() << "Received sync message 'entities' is not an array" << std::endl;
        return;
    }
    
    const auto& entities = document["entities"].GetArray();
    for (const auto& entity : entities) {
        if (!entity.HasMember("entity_id") || !entity["entity_id"].IsUint64()) {
            log_error() << "Entity missing required 'entity_id' field" << std::endl;
            continue;
        }
        
        if (!entity.HasMember("variants") || !entity["variants"].IsArray()) {
            log_error() << "Entity missing required 'variants' field" << std::endl;
            continue;
        }

        uint64_t entity_id = entity["entity_id"].GetUint64();
        bool found = false;
        
        for (auto& entity_doc : m_entities) {
            const auto& existing_doc = entity_doc.get_document();
            if (existing_doc.HasMember("entity_id") &&
                existing_doc["entity_id"].GetUint64() == entity_id) {
                rapidjson::Document new_doc;
                new_doc.CopyFrom(entity, new_doc.GetAllocator());
                entity_doc.set_document(std::move(new_doc));
                found = true;
                break;
            }
        }
        
        if (!found) {
            log_error() << "Entity with ID " << entity_id << " not found in entity list" << std::endl;
        }
    }
}

void EntityList::backup_entities() {
    std::filesystem::path backupDir = BACKUP_DIR;

    std::error_code ec;
    if (!std::filesystem::exists(backupDir)) {
        if (!std::filesystem::create_directory(backupDir, ec)) {
            log_error() << "Failed to create backup directory: " << ec.message() << std::endl;
            return;
        }
    }

    for (const auto& entry : std::filesystem::directory_iterator(backupDir, ec)) {
        if (ec) {
            log_error() << "Error iterating backup directory: " << ec.message() << std::endl;
            return;
        }

        if (!std::filesystem::remove(entry.path(), ec)) {
            log_error() << "Failed to remove old backup file: " << entry.path() << ", error: " << ec.message() << std::endl;
        }
    }

    for (const auto& entity : m_entities) {
        if(entity.is_dead()) continue;

        std::string name = entity.get_name();

        if(name.empty()) {
            log_error() << "Attempt to process unnamed entity" << std::endl;
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
            log_error() << "Error checking if path exists: " << path << ", error: " << ec.message() << std::endl;
            return;
        }

        log_error() << "Path does not exist: " << path << std::endl;
        return;
    }

    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path, ec)) {
        if (ec) {
            log_error() << "Error iterating directory: " << path << ", error: " << ec.message() << std::endl;
            return;
        }

        if (!entry.is_regular_file(ec) || entry.path().extension() != ENTITY_EXTENSION) {
            if (ec) {
                log_error() << "Error checking if path is regular file: " << entry.path() << ", error: " << ec.message() << std::endl;
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
            log_error() << "Error checking path: " << path << ", error: " << ec.message() << std::endl;
        } else {
            log_error() << "Path does not exist or is not a regular file: " << path << std::endl;
        }
        return;
    }

    std::string file_name = path.stem().string();

    if(file_name.empty()) {
        log_error() << "Error: Loading unnamed entity is not allowed" << std::endl;
        return;
    }

    auto& entity = m_entities.emplace_back(EntityDocument(std::move(file_name)));

    entity.load_from_file(path);
}
