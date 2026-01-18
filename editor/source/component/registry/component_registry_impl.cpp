module;

#include <algorithm>
#include <string> // IWYU pragma: keep
#include <filesystem>
#include <thread>

module zeytin.component.registry;

import zeytin.component.document.conversion;
import zeytin.component.instance;
import zeytin.common.guid;
import zeytin.filewatcher;
import zeytin.resource;

namespace {
	HashMap<ComponentID, Component> g_component_id_to_component;
	HashMap<ComponentID, List<ComponentInstanceID>> g_instances_by_component;
	HashMap<ComponentInstanceID, ComponentInstance> g_instances;
	HashMap<ComponentDocumentID, ComponentDocument> g_documents;
		
	FileWatcher* g_file_watcher = nullptr;
	
	// NOTE: ComponentID == ComponentDocumentID (they use the same stable hash)
}

static Maybe<ComponentDocumentID> find_document_id(const String& name);
static ComponentDocumentID generate_document_id(const String& name);

static void load_component(PathView path);
static void load_all_components();
static void start_watching();

ComponentID sync_component(const ComponentDocument& document) {
    Component component = component_from_document(document);

    g_component_id_to_component[component.id] = std::move(component);

    return component.id;
}

bool remove_component(const ComponentDocumentID document_id) {
    // ComponentID == ComponentDocumentID
    const ComponentID component_id = document_id;
    
    auto it = g_component_id_to_component.find(component_id);
    if (it == g_component_id_to_component.end()) {
        return false;
    }

    g_component_id_to_component.erase(it);

    auto instance_list_it = g_instances_by_component.find(component_id);
    if (instance_list_it != g_instances_by_component.end()) {
        for (const auto instance_id : instance_list_it->second) {
            g_instances.erase(instance_id);
        }
        g_instances_by_component.erase(instance_list_it);
    }

    return true;
}

MaybeRef<Component> get_component(const ComponentID component_id) {
    auto it = g_component_id_to_component.find(component_id);
    if (it != g_component_id_to_component.end()) {
        return std::ref(it->second);
    }
    return {};
}

MaybeRef<const Component> get_component_const(const ComponentID component_id) {
    auto it = g_component_id_to_component.find(component_id);
    if (it != g_component_id_to_component.end()) {
        return std::cref(it->second);
    }
    return {};
}

MaybeRef<Component> get_component_from_document(const ComponentDocumentID document_id) {
    return get_component(document_id);
}

MaybeRef<const Component> get_component_from_document_const(const ComponentDocumentID document_id) {
    return get_component_const(document_id);
}

MaybeRef<Component> get_component_by_name(const String& name) {
    for (auto& [id, component] : g_component_id_to_component) {
        if (component.type == name) {
            return std::ref(component);
        }
    }
    return {};
}

MaybeRef<const Component> get_component_by_name_const(const String& name) {
    for (const auto& [id, component] : g_component_id_to_component) {
        if (component.type == name) {
            return std::cref(component);
        }
    }
    return {};
}

List<ComponentID> get_component_ids() {
    List<ComponentID> ids;
    ids.reserve(g_component_id_to_component.size());
    for (const auto& [id, component] : g_component_id_to_component) {
        ids.push_back(id);
    }
    return ids;
}

List<ComponentDocumentID> get_document_ids() {
    List<ComponentDocumentID> ids;
    ids.reserve(g_documents.size());
    for (const auto& [id, doc] : g_documents) {
        ids.push_back(id);
    }
    return ids;
}

MaybeRef<ComponentInstance> create_instance(const ComponentID component_id) {
	ComponentInstance instance {
		.id = generate_unique_id(),
		.component_id = component_id,
		.overrides = {}
	};

    auto [it, inserted] = g_instances.emplace(instance.id, std::move(instance));
    if (!inserted) {
        return {};
    }

    g_instances_by_component[component_id].push_back(instance.id);

    return std::ref(it->second);
}

MaybeRef<ComponentInstance> create_instance(const Component& component) {
	return create_instance(component.id);
}

MaybeRef<ComponentInstance> get_instance(const ComponentInstanceID id) {
    auto it = g_instances.find(id);
    if (it != g_instances.end()) {
        return std::ref(it->second);
    }
    return {};
}

MaybeRef<const ComponentInstance> get_instance_const(const ComponentInstanceID id) {
    auto it = g_instances.find(id);
    if (it != g_instances.end()) {
        return std::cref(it->second);
    }
    return {};
}

bool destroy_instance(const ComponentInstanceID id) {
    auto it = g_instances.find(id);
    if (it == g_instances.end()) {
        return false;
    }

    const ComponentID component_id = it->second.component_id;

    auto list_it = g_instances_by_component.find(component_id);
    if (list_it != g_instances_by_component.end()) {
        auto& instances = list_it->second;
        instances.erase(std::remove(instances.begin(), instances.end(), id), instances.end());
        if (instances.empty()) {
            g_instances_by_component.erase(list_it);
        }
    }

    g_instances.erase(it);
    return true;
}

const List<ComponentInstanceID>& get_instances_for(const ComponentID component_id) {
    static const List<ComponentInstanceID> empty;
    auto it = g_instances_by_component.find(component_id);
    if (it != g_instances_by_component.end()) {
        return it->second;
    }
    return empty;
}

List<ComponentInstanceID> get_instances_for_copy(const ComponentID component_id) {
    auto it = g_instances_by_component.find(component_id);
    if (it != g_instances_by_component.end()) {
        return it->second;
    }
    return {};
}

void initialize_component_registry() {
    load_all_components();
    start_watching();
}

MaybeRef<ComponentDocument> get_document(const ComponentDocumentID document_id) {
    auto it = g_documents.find(document_id);
    if (it != g_documents.end()) {
        return std::ref(it->second);
    }
    return {};
}

MaybeRef<const ComponentDocument> get_document_const(const ComponentDocumentID document_id) {
    auto it = g_documents.find(document_id);
    if (it != g_documents.end()) {
        return std::cref(it->second);
    }
    return {};
}

MaybeRef<ComponentDocument> get_document_by_name(const String& name) {
    auto doc_id_opt = find_document_id(name);
    if (!doc_id_opt) {
        return {};
    }
    auto it = g_documents.find(doc_id_opt.value());
    if (it != g_documents.end()) {
        return std::ref(it->second);
    }
    return {};
}

MaybeRef<const ComponentDocument> get_document_by_name_const(const String& name) {
    auto doc_id_opt = find_document_id(name);
    if (!doc_id_opt) {
        return {};
    }
    auto it = g_documents.find(doc_id_opt.value());
    if (it != g_documents.end()) {
        return std::cref(it->second);
    }
    return {};
}

Maybe<ComponentDocumentID> get_document_id_for_component(const ComponentID component_id) {
    if (g_component_id_to_component.contains(component_id)) {
        return component_id;
    }
    return {};
}

MaybeRef<ComponentDocument> get_document_from_component(const ComponentID component_id) {
    return get_document(component_id);
}

MaybeRef<const ComponentDocument> get_document_from_component_const(const ComponentID component_id) {
    return get_document_const(component_id);
}

// -------------------------------------

static Maybe<ComponentDocumentID> find_document_id(const String& name) {
	for (const auto& [id, doc] : g_documents) {
        if (doc.get_name() == name) {
            return id;
        }
    }
    return {};
}

static ComponentDocumentID generate_document_id(const String& name) {
    return generate_stable_id(name, 0);
}

static void load_component(PathView path) {
	String name = path.stem().string();
	auto doc_id_opt = find_document_id(name);
	ComponentDocumentID doc_id;
	
	if (doc_id_opt) {
		doc_id = doc_id_opt.value();
		auto it = g_documents.find(doc_id);
		if (it != g_documents.end()) {
			it->second.set_alive();
			it->second.load_from_file();
			sync_component(it->second);
		}
	} else {
		doc_id = generate_document_id(name);
		auto [inserted_it, success] = g_documents.emplace(doc_id, ComponentDocument(std::move(name)));
		if (success) {
			inserted_it->second.set_id(doc_id);
			inserted_it->second.load_from_file();
			sync_component(inserted_it->second);
		}
	}
}

static void load_all_components() {
	g_documents.clear();

	for(const auto& entry : std::filesystem::directory_iterator(ResourceManager::get().get_components_paths())) {
		if(!entry.is_regular_file() || entry.path().extension() != ".component") {
			continue;
		}

		Path file_path = entry.path();
		String name = file_path.stem().string();
		ComponentDocumentID doc_id = generate_document_id(name);

		auto [it, success] = g_documents.emplace(doc_id, ComponentDocument(std::move(name)));
		if (success) {
			it->second.set_id(doc_id);
		}
	}

	for(auto& [id, doc] : g_documents) {
		doc.load_from_file();
		sync_component(doc);
	}
}

static void start_watching() {
	if (!g_file_watcher) {
		g_file_watcher = new FileWatcher(ResourceManager::get().get_components_paths(), std::chrono::milliseconds(500));
	}

	g_file_watcher->add_callback({".component"}, [](PathView path, const FileEvent event) {
		if(event == FileEvent::Modified || event == FileEvent::Created) {
			load_component(path);
		}
		else if(event == FileEvent::Deleted) {
			String name = path.stem().string();
			auto doc_id_opt = find_document_id(name);
			if (doc_id_opt) {
				ComponentDocumentID doc_id = doc_id_opt.value();
				remove_component(doc_id);
				g_documents.erase(doc_id);
			}
		}
	});
	
	std::thread watcher_thread([](){ 
		g_file_watcher->start();
	});

	watcher_thread.detach(); 
}
