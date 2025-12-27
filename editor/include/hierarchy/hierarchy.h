#pragma once

#include "entity/entity_document.h"
#include "variant/variant_document.h"
#include <vector>
#include <map>

class Hierarchy final {
public: 
    Hierarchy(std::vector<EntityDocument>& entities, std::vector<VariantDocument>& variants);
    void update();

private:
    void render_save_controls();
    void render_create_entity();
    void create_new_entity(const char* name);
    void render_entity(EntityDocument& entity);
    void handle_entity_context_menu(EntityDocument& entity_document, uint64_t entity_id);
    void render_variant(rapidjson::Document& document, rapidjson::Value& variant, 
                       int index, uint64_t entity_id);
    void render_object(rapidjson::Document& document, rapidjson::Value& object, 
                      uint64_t entity_id, const std::string& variant_type, 
                      const std::string& parent_path = "");
    
    void render_int_field(rapidjson::Document& document, rapidjson::Value& value,
                         uint64_t entity_id, const std::string& variant_type,
                         const std::string& key, const std::string& current_path,
                         const std::string& uniqueId, std::map<std::string, bool>& editingField);
    
    void render_float_field(rapidjson::Document& document, rapidjson::Value& value,
                           uint64_t entity_id, const std::string& variant_type,
                           const std::string& key, const std::string& current_path,
                           const std::string& uniqueId, std::map<std::string, bool>& editingField);
    
    void render_bool_field(rapidjson::Value& value, uint64_t entity_id,
                          const std::string& variant_type, const std::string& key,
                          const std::string& current_path);
    
    void render_string_field(rapidjson::Document& document, rapidjson::Value& value,
                            uint64_t entity_id, const std::string& variant_type,
                            const std::string& key, const std::string& current_path,
                            const std::string& uniqueId, std::map<std::string, bool>& editingField);
    
    void render_array_field(rapidjson::Document& document, rapidjson::Value& value,
                           uint64_t entity_id, const std::string& variant_type,
                           const std::string& key, const std::string& current_path,
                           std::map<std::string, bool>& editingField);

    void render_add_variant_menu(EntityDocument& entity_document);
    bool check_variant_exists(const EntityDocument& entity_document, const std::string& variant_name);
    void update_recent_variants(std::vector<std::string>& recent_variants,
                              const std::string& variant_name,
                              int max_recent);
    void add_variant_to_entity(EntityDocument& entity_document, VariantDocument& variant_document);

    void add_required_variants_to_entity(EntityDocument& entity_document, const std::string& variant_type);
    void save_all_entities();
    void subscribe_events();

    std::vector<EntityDocument>& m_entities;
    std::vector<VariantDocument>& m_variants;
};
