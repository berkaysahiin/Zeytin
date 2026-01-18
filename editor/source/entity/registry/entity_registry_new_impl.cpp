module;

#include <algorithm>
#include <filesystem>
#include <thread>

module zeytin.entity.registry.runtime;

import zeytin.common.guid;
import zeytin.entity;
import zeytin.entity.document;
import zeytin.entity.document.conversion;
import zeytin.filewatcher;
import zeytin.logger;
import zeytin.resource;
import zeytin.utility.typedefs;

namespace {
	// TODO; Introduce "Scene" for proper Scene support
	// HashMap<SceneID, List<EntityID>>
    HashMap<EntityDocumentID, EntityDocument> g_documents;
    HashMap<EntityID, Entity> g_entities;
    HashMap<EntityID, EntityDocumentID> g_entity_to_document;
    HashMap<EntityDocumentID, EntityID> g_document_to_entity;

    FileWatcher* g_file_watcher = nullptr;
    Level g_current_level;

    EntityDocumentID generate_document_id(ConstRef<String> name, ConstRef<Level> level);

    void clear_registry();
    void register_entity(EntityDocumentID doc_id, EntityDocument&& document);
    void load_entity_file(ConstRef<Path> path);
    void load_entities_from_level(ConstRef<Level> level);
    void remove_entity_by_path(ConstRef<Path> path);
    void start_watching(ConstRef<Level> level);
}

#define ENTITY_EXTENSION ".entity"

// public API implementations
void initialize_entity_runtime_registry() {
    List<Level> levels = runtime_get_available_levels();
    if (levels.empty()) {
        return;
    }

    runtime_load_level(levels.front());
}

void shutdown_entity_runtime_registry() {
    if (g_file_watcher) {
        g_file_watcher->stop();
        delete g_file_watcher;
        g_file_watcher = nullptr;
    }

    clear_registry();
}

void runtime_load_level(const Level& level) {
    g_current_level = level;
    load_entities_from_level(level);
    start_watching(level);
}

const Level& runtime_get_current_level() {
    return g_current_level;
}

List<Level> runtime_get_available_levels() {
    List<Level> levels;
    const Path levels_path = ResourceManager::get().get_resource_subdir("levels");

    for (const auto& entry : fs::directory_iterator(levels_path)) {
        if (!entry.is_directory()) {
            continue;
        }

        levels.emplace_back(entry.path().filename().string(), entry.path());
    }

    std::sort(levels.begin(), levels.end(), [](const Level& left, const Level& right) {
        return left.name < right.name;
    });

    return levels;
}

List<EntityID> runtime_get_entity_ids() {
    List<EntityID> ids;
    ids.reserve(g_entities.size());
    for (const auto& [id, entity] : g_entities) {
        ids.push_back(id);
    }
    return ids;
}

MaybeRef<Entity> runtime_get_entity(EntityID entity_id) {
    auto it = g_entities.find(entity_id);
    if (it == g_entities.end()) {
        return {};
    }

    return std::ref(it->second);
}

MaybeRef<const Entity> runtime_get_entity_const(EntityID entity_id) {
    auto it = g_entities.find(entity_id);
    if (it == g_entities.end()) {
        return {};
    }

    return std::cref(it->second);
}

List<EntityDocumentID> runtime_get_entity_document_ids() {
    List<EntityDocumentID> ids;
    ids.reserve(g_documents.size());
    for (const auto& [id, doc] : g_documents) {
        ids.push_back(id);
    }
    return ids;
}

MaybeRef<EntityDocument> runtime_get_entity_document(const EntityDocumentID document_id) {
    auto it = g_documents.find(document_id);
    if (it == g_documents.end()) {
        return {};
    }

    return std::ref(it->second);
}

MaybeRef<const EntityDocument> runtime_get_entity_document_const(const EntityDocumentID document_id) {
    auto it = g_documents.find(document_id);
    if (it == g_documents.end()) {
        return {};
    }

    return std::cref(it->second);
}

Maybe<EntityDocumentID> runtime_get_document_id_for_entity(const EntityID entity_id) {
    auto it = g_entity_to_document.find(entity_id);
    if (it == g_entity_to_document.end()) {
        return {};
    }

    return it->second;
}

Maybe<EntityID> runtime_get_entity_id_for_document(const EntityDocumentID document_id) {
    auto it = g_document_to_entity.find(document_id);
    if (it == g_document_to_entity.end()) {
        return {};
    }

    return it->second;
}

// =================================================================================================================================

namespace {

EntityDocumentID generate_document_id(ConstRef<String> name, ConstRef<Level> level) {
    String key = level.name + "/" + name;
    return generate_stable_id(key, 0);
}

void clear_registry() {
    g_entities.clear();
    g_documents.clear();
    g_entity_to_document.clear();
    g_document_to_entity.clear();
}

void register_entity(EntityDocumentID doc_id, EntityDocument&& document) {
    if (!document.is_valid()) {
        return;
    }

    const Entity entity = entity_from_document(document);
    const EntityID entity_id = entity.id;
    
    g_entities[entity_id] = entity;
    g_documents.emplace(doc_id, std::move(document));
    g_entity_to_document[entity_id] = doc_id;
    g_document_to_entity[doc_id] = entity_id;
}

void load_entity_file(ConstRef<Path> path) {
    if (!fs::exists(path)) {
        return;
    }

    const String name = path.stem().string();
    if (name.empty()) {
        return;
    }

    EntityDocument document(name);
    document.set_file_path(path);
    document.load_from_file();

	// TODO Should move this to entity document. Generate document ID based on full path which is unique + contains level path anyway.
    const EntityDocumentID doc_id = generate_document_id(name, g_current_level);
    document.set_document_id(doc_id);
    register_entity(doc_id, std::move(document));
}

void load_entities_from_level(const Level& level) {
    clear_registry();

    if (!level.is_valid()) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(level.path)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        if (entry.path().extension() != ENTITY_EXTENSION) {
            continue;
        }

        load_entity_file(entry.path());
    }
}

void remove_entity_by_path(ConstRef<Path> path) {
    if (!path.has_filename()) {
        return;
    }

    auto it = std::find_if(g_documents.begin(), g_documents.end(),
        [&path](const auto& pair) {
            return pair.second.get_file_path() == path;
        });

    if (it == g_documents.end()) {
        return;
    }

    const EntityDocumentID doc_id = it->first;
    auto entity_id_opt = runtime_get_entity_id_for_document(doc_id);
    if (entity_id_opt) {
        g_entities.erase(entity_id_opt.value());
        g_entity_to_document.erase(entity_id_opt.value());
    }

    g_document_to_entity.erase(doc_id);
    g_documents.erase(it);
}

void start_watching(const Level& level) {
    if (g_file_watcher) {
        g_file_watcher->stop();
        delete g_file_watcher;
        g_file_watcher = nullptr;
    }

    if (!level.is_valid()) {
        return;
    }

    g_file_watcher = new FileWatcher(level.path, std::chrono::milliseconds(500));
    g_file_watcher->add_callback({ENTITY_EXTENSION}, [](ConstRef<Path> path, const FileEvent event) {
        if (event == FileEvent::Deleted) {
            remove_entity_by_path(path);
            return;
        }

        load_entity_file(path);
    });

    std::thread watcher_thread([]() {
        g_file_watcher->start();
    });

    watcher_thread.detach();
}

}
