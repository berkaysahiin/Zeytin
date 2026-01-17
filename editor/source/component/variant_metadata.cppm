module;

#include <string>
#include <filesystem>
#include <optional>
#include <memory>
#include <unordered_map>

export module zeytin.variant.metadata;
import zeytin.singleton;

export class VariantMetadata : public Singleton<VariantMetadata> {
    friend class Singleton<VariantMetadata>;
public:
    ~VariantMetadata();
    
    bool has_annotation(const std::string& variant_name, 
                       const std::string& property_name, 
                       const std::string& annotation_key) const;

    bool has_any_annotation(const std::string& variant_name, 
                       const std::string& property_name) const;
    
    std::optional<std::string> get_annotation(const std::string& variant_name,
                                              const std::string& property_name,
                                              const std::string& annotation_key) const;

	bool add_annotation(const std::string& variant_name,
                                              const std::string& property_name,
                                              std::string annotation_key, std::string annotation_value);
    
    std::unordered_map<std::string, std::string> get_all_annotations(
        const std::string& variant_name,
        const std::string& property_name) const;
    
    void load_from_component_files(const std::filesystem::path& components_dir);
    
private:
    VariantMetadata();
    
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
