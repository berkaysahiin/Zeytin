module;

#include <vector>
#include <filesystem>

export module zeytin.component.list;
import zeytin.component.document;
import zeytin.filewatcher;

export class ComponentList final {
public:
    ComponentList();

    inline std::vector<ComponentDocument>& get_variants() { return m_variants;}
    inline const std::vector<ComponentDocument>& get_variants() const { return m_variants;}

private:
    void load_variants();
    void start_watching();
    void load_variant(const std::filesystem::path& path);

    std::vector<ComponentDocument> m_variants;
    FileWatcher m_variant_watcher;
};
