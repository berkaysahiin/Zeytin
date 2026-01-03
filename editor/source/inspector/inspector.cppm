module;

#include <string>
#include <map>
#include <cstdint>
#include <vector>
#include "rapidjson/document.h"

export module zeytin.inspector;
import zeytin.selection;
import zeytin.entity.document;
import zeytin.variant.document;
import zeytin.variant.list;

export class Inspector final {
public:
    Inspector(std::vector<VariantDocument>& variants);
    void render();

private:
    void render_entity_header(EntityDocument& entity);
    void render_variants(EntityDocument& entity);
    void render_variant(rapidjson::Document& document, rapidjson::Value& variant, 
                        int index, uint64_t entity_id);
    void render_variant_header(rapidjson::Value& variant, int index, uint64_t entity_id,
                               rapidjson::Value& variants_array);

    void render_object(rapidjson::Document& document, rapidjson::Value& object,
                       uint64_t entity_id, const std::string& variant_type,
                       const std::string& parent_path = "");

    void render_int_field(rapidjson::Document& document, rapidjson::Value& value,
                          uint64_t entity_id, const std::string& variant_type,
                          const std::string& key, const std::string& current_path);

    void render_float_field(rapidjson::Document& document, rapidjson::Value& value,
                            uint64_t entity_id, const std::string& variant_type,
                            const std::string& key, const std::string& current_path);

    void render_bool_field(rapidjson::Value& value, uint64_t entity_id,
                           const std::string& variant_type, const std::string& key,
                           const std::string& current_path);

    void render_string_field(rapidjson::Document& document, rapidjson::Value& value,
                             uint64_t entity_id, const std::string& variant_type,
                             const std::string& key, const std::string& current_path);

    void render_array_field(rapidjson::Document& document, rapidjson::Value& value,
                            uint64_t entity_id, const std::string& variant_type,
                            const std::string& key, const std::string& current_path);

    void render_add_component_button(EntityDocument& entity, uint64_t entity_id);
    void add_variant_to_entity(EntityDocument& entity, VariantDocument& variant);

    std::vector<VariantDocument>& m_variants;
    std::map<std::string, bool> m_editing_field;
};
