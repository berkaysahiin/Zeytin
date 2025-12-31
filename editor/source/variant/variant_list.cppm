module;

#include <vector>
#include <filesystem>

export module zeytin.variant.list;
import zeytin.variant.document;
import zeytin.filewatcher;

export class VariantList final {
public:
    VariantList();

    inline std::vector<VariantDocument>& get_variants() { return m_variants;}

private:
    void load_variants();
    void start_watching();
    void load_variant(const std::filesystem::path& path);


    std::vector<VariantDocument> m_variants;
    FileW m_variant_watcher;
};
