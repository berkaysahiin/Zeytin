module;

#include <filesystem>
#include <thread>

module zeytin.variant.list;
import zeytin.filewatcher;
import zeytin.resource;

VariantList::VariantList() : m_variant_watcher(ResourceManager::get().get_components_paths(), std::chrono::milliseconds(500)) {
    load_variants();
    start_watching();
}

void VariantList::load_variants() {
    m_variants.clear();

    for(const auto& entry : std::filesystem::directory_iterator(ResourceManager::get().get_components_paths())) {
        if(!entry.is_regular_file() || entry.path().extension() != ".component") {
            continue;
        }

        std::filesystem::path file_path = entry.path();
        std::string name = file_path.stem().string();

        m_variants.emplace_back<VariantDocument>(std::move(name));
    }

    for(auto& variant : m_variants) {
        variant.load_from_file();
    }
}

void VariantList::load_variant(const std::filesystem::path& path) {
    std::string name = path.stem().string();

    auto it = std::ranges::find_if(m_variants.begin(), m_variants.end(),
                          [&name](const auto& variant) {
                              return variant.get_name() == name;
                          });

    if (it != m_variants.end()) {
        it->set_alive();
        it->load_from_file();
    } else {
        auto& rv = m_variants.emplace_back(VariantDocument(std::move(name)));
        rv.load_from_file();
    }
}

void VariantList::start_watching() {
    m_variant_watcher.add_callback({ ".component"}, [this](const std::filesystem::path& path, const FileEvent event) {
        if(event == FileEvent::Modified || event == FileEvent::Created) {
            load_variant(path);
        }
        else if(event == FileEvent::Deleted) {
            std::string name = path.stem().string();
            for(auto& variant : m_variants) {
                if(variant.get_name() == name) {
                    variant.mark_dead();
                }
            }
        }
    });
    
    std::thread watcher_thread([this]() {
        m_variant_watcher.start();
    });

    watcher_thread.detach(); 
}


